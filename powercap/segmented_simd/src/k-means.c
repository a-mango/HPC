#include "k-means.h"

#include <immintrin.h>
#include <limits.h>
#include <math.h>
#include <powercap-rapl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "perf_manager.h"


#define R_OFFSET 0
#define G_OFFSET 1
#define B_OFFSET 2
#define ALIGN 32

// Xorshift32 PRNG
static inline uint32_t xor_rand() {
    static uint32_t y = 2463534242UL;
    y ^= (y << 13);
    y ^= (y >> 17);
    return (y ^= (y << 15));
}

static inline void *alloc(size_t align, size_t size) {
    void *ptr = NULL;
    if (posix_memalign(&ptr, align, size) != 0 || !ptr) {
        fprintf(stderr, "Aligned allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void kmeans_pp(struct img_t *image, int num_clusters, uint8_t *centers) {
    const int comps   = image->components;
    const int npixels = image->width * image->height;

    // Initial random center
    int first_idx = (xor_rand() % npixels) * comps;
    memcpy(centers, image->data + first_idx, comps);

    float         *distances    = alloc(ALIGN, npixels * sizeof(float));
    const uint8_t *first_center = centers;

    // Precompute distances from the first center
    for (int i = 0; i < npixels; ++i) {
        const uint8_t *px = image->data + i * comps;
        int            dr = px[R_OFFSET] - first_center[R_OFFSET];
        int            dg = px[G_OFFSET] - first_center[G_OFFSET];
        int            db = px[B_OFFSET] - first_center[B_OFFSET];
        distances[i]      = dr * dr + dg * dg + db * db;
    }

    // Select remaining centers
    for (int c = 1; c < num_clusters; ++c) {
        float total = 0;
        for (int i = 0; i < npixels; ++i) {
            total += distances[i];
        }

        float threshold = (float)xor_rand() / UINT32_MAX * total;
        int   sel       = 0;
        while (sel < npixels && threshold > distances[sel]) {
            threshold -= distances[sel++];
        }
        sel = sel < npixels ? sel : npixels - 1;

        memcpy(centers + c * comps, image->data + sel * comps, comps);
        const uint8_t *new_center = centers + c * comps;

        // Update distances
        for (int i = 0; i < npixels; ++i) {
            const uint8_t *pix  = image->data + i * comps;
            int            dr   = pix[R_OFFSET] - new_center[R_OFFSET];
            int            dg   = pix[G_OFFSET] - new_center[G_OFFSET];
            int            db   = pix[B_OFFSET] - new_center[B_OFFSET];
            float          dist = dr * dr + dg * dg + db * db;

            if (dist < distances[i]) {
                distances[i] = dist;
            }
        }
    }
    free(distances);
}

void kmeans(struct img_t *image, int n_clusters) {
    // Powercap setup
    powercap_rapl_pkg  pkg;
    powercap_rapl_zone zone = POWERCAP_RAPL_ZONE_PSYS;
    uint64_t           energy_before, energy_after;
    powercap_rapl_init(0, &pkg, 1);
    sleep(1);  // Allow time for energy counter initialization
    powercap_rapl_get_energy_uj(&pkg, zone, &energy_before);

    // Perf setup
    PerfManager pmon;
    PerfManager_init(&pmon);
    PerfManager_resume(&pmon);

    const int n_comp = image->components;
    const int n_px   = image->width * image->height;
    const int n_pad  = ((n_clusters + 7) / 8) * 8;  // Pad to 8

    // Aligned allocs
    uint8_t *centers     = alloc(ALIGN, n_clusters * n_comp);
    uint8_t *centers_r   = alloc(ALIGN, n_pad);
    uint8_t *centers_g   = alloc(ALIGN, n_pad);
    uint8_t *centers_b   = alloc(ALIGN, n_pad);
    int     *assignments = alloc(ALIGN, n_px * sizeof(int));

    kmeans_pp(image, n_clusters, centers);

    // Transpose centers to struct of arrays
    for (int c = 0; c < n_clusters; ++c) {
        centers_r[c] = centers[c * n_comp + R_OFFSET];
        centers_g[c] = centers[c * n_comp + G_OFFSET];
        centers_b[c] = centers[c * n_comp + B_OFFSET];
    }

    for (int c = n_clusters; c < n_pad; ++c) {
        centers_r[c] = UINT8_MAX;
        centers_g[c] = UINT8_MAX;
        centers_b[c] = UINT8_MAX;
    }

    // Vectorized assignment of pixels to clusters
    for (int i = 0; i < n_px; ++i) {
        const uint8_t *pix = image->data + i * n_comp;
        const uint8_t  pr = pix[R_OFFSET], pg = pix[G_OFFSET], pb = pix[B_OFFSET];

        __m256i min_dist    = _mm256_set1_epi32(INT_MAX);
        __m256i best_idx    = _mm256_setzero_si256();
        __m256i current_idx = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);

        for (int c = 0; c < n_pad; c += 8) {
            // Load 8 elements using 64-bit load
            __m128i cr8 = _mm_loadl_epi64((__m128i *)(centers_r + c));
            __m128i cg8 = _mm_loadl_epi64((__m128i *)(centers_g + c));
            __m128i cb8 = _mm_loadl_epi64((__m128i *)(centers_b + c));

            // Expand to 32-bit integers
            __m256i cr = _mm256_cvtepu8_epi32(cr8);
            __m256i cg = _mm256_cvtepu8_epi32(cg8);
            __m256i cb = _mm256_cvtepu8_epi32(cb8);

            // Broadcast pixel values
            __m256i vr = _mm256_set1_epi32(pr);
            __m256i vg = _mm256_set1_epi32(pg);
            __m256i vb = _mm256_set1_epi32(pb);

            // Compute differences
            __m256i dr = _mm256_sub_epi32(cr, vr);
            __m256i dg = _mm256_sub_epi32(cg, vg);
            __m256i db = _mm256_sub_epi32(cb, vb);

            // Square differences
            dr = _mm256_mullo_epi32(dr, dr);
            dg = _mm256_mullo_epi32(dg, dg);
            db = _mm256_mullo_epi32(db, db);

            __m256i dist = _mm256_add_epi32(dr, _mm256_add_epi32(dg, db));

            // Update minimums
            __m256i mask = _mm256_cmpgt_epi32(min_dist, dist);
            min_dist     = _mm256_blendv_epi8(min_dist, dist, mask);
            best_idx     = _mm256_blendv_epi8(best_idx, current_idx, mask);

            // Increment the index by 8 to process the next cluster
            current_idx = _mm256_add_epi32(current_idx, _mm256_set1_epi32(8));
        }

        // Extract the minimum value
        int distances[8] __attribute__((aligned(32)));
        int indices[8] __attribute__((aligned(32)));
        _mm256_store_si256((__m256i *)distances, min_dist);
        _mm256_store_si256((__m256i *)indices, best_idx);

        int best = 0;
        for (int j = 1; j < 8; ++j) {
            if (distances[j] < distances[best]) {
                best = j;
            }
        }
        assignments[i] = indices[best] < n_clusters ? indices[best] : 0;
    }

    // Update the clusters and compute the centers
    ClusterData *cd = alloc(ALIGN, n_clusters * sizeof(ClusterData));
    memset(cd, 0, n_clusters * sizeof(ClusterData));

    for (int i = 0; i < n_px; ++i) {
        int c = assignments[i];
        if (c >= n_clusters) {
            c = 0;
        }
        const uint8_t *pix = image->data + i * n_comp;
        cd[c].count++;
        cd[c].sum_r += pix[R_OFFSET];
        cd[c].sum_g += pix[G_OFFSET];
        cd[c].sum_b += pix[B_OFFSET];
    }

    // Update centers
    for (int c = 0; c < n_clusters; ++c) {
        if (cd[c].count > 0) {
            centers[c * n_comp + R_OFFSET] = (cd[c].sum_r + cd[c].count / 2) / cd[c].count;
            centers[c * n_comp + G_OFFSET] = (cd[c].sum_g + cd[c].count / 2) / cd[c].count;
            centers[c * n_comp + B_OFFSET] = (cd[c].sum_b + cd[c].count / 2) / cd[c].count;
        }
    }

    // Finally copy the pixel
    for (int i = 0; i < n_px; ++i) {
        int c = assignments[i];
        if (c >= n_clusters) {
            c = 0;
        }
        uint8_t       *pix    = image->data + i * n_comp;
        const uint8_t *center = centers + c * n_comp;
        memcpy(pix, center, n_comp);
    }

    // Powrecap and Perf cleanup
    PerfManager_pause(&pmon);
    powercap_rapl_get_energy_uj(&pkg, zone, &energy_after);
    printf("[PowerCap] Energy used: %.4f J\n", (energy_after - energy_before) / 1e6);
    powercap_rapl_destroy(&pkg);

    free(cd);
    free(assignments);
    free(centers_r);
    free(centers_g);
    free(centers_b);
    free(centers);
}

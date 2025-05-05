#include "k-means.h"

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define R_OFFSET 0
#define G_OFFSET 1
#define B_OFFSET 2
#define CACHE_LINE 64
#define UNROLL 4

// Fast Xorshift64* PRNG
static inline uint32_t xorshift64s() {
    static uint64_t xorshift_state = 88172645463325252ULL;
    xorshift_state ^= xorshift_state >> 12;
    xorshift_state ^= xorshift_state << 25;
    xorshift_state ^= xorshift_state >> 27;
    return (xorshift_state * 0x2545F4914F6CDD1DULL) >> 32;
}

// Cache-aligned allocation
static inline void *alloc(size_t align, size_t size) {
    void *ptr;
    posix_memalign(&ptr, align, size);
    return ptr;
}

void kmeans_pp(struct img_t *image, int num_clusters, uint8_t *centers) {
    const int comps = image->components;
    const int npixels = image->width * image->height;
    const size_t pixel_size = comps * sizeof(uint8_t);

    // Align critical data structures to cache lines
    uint32_t *distances = alloc(CACHE_LINE, npixels * sizeof(uint32_t));

    // Initialize first center using cache-line aligned random access
    const int first_idx = (xorshift64s() % npixels) * comps;
    memcpy(centers, &image->data[first_idx], pixel_size);

    // Precompute squared distances in cache-friendly blocks
    const uint8_t *first_center = centers;
#pragma unroll(4)
    for (int i = 0; i < npixels; i += CACHE_LINE / sizeof(uint32_t)) {
        const int end = (i + CACHE_LINE / sizeof(uint32_t)) < npixels
                            ? i + CACHE_LINE / sizeof(uint32_t)
                            : npixels;
        for (int j = i; j < end; j++) {
            const uint8_t *pix = &image->data[j * comps];
            const int dr = pix[R_OFFSET] - first_center[R_OFFSET];
            const int dg = pix[G_OFFSET] - first_center[G_OFFSET];
            const int db = pix[B_OFFSET] - first_center[B_OFFSET];
            distances[j] = dr * dr + dg * dg + db * db;
        }
    }

    // Select subsequent centers with weighted probability
    for (int c = 1; c < num_clusters; c++) {
        // Parallel sum using 64-bit accumulator
        uint64_t total = 0;
        for (int i = 0; i < npixels; i += 4) {
            total += distances[i] + distances[i + 1] + distances[i + 2] + distances[i + 3];
        }

        // Binary search inspired selection with early exit
        const uint64_t threshold = (xorshift64s() * total) >> 32;
        uint64_t accum = 0;
        int sel = 0;
        while (sel < npixels && accum < threshold) {
            accum += distances[sel];
            sel += (threshold - accum) > (threshold >> 8) ? 16 : 1;
        }
        sel = sel < npixels ? sel : npixels - 1;

        // Update centers and distances
        const uint8_t *new_center = &image->data[sel * comps];
#pragma unroll(4)
        for (int i = 0; i < npixels; i += CACHE_LINE / sizeof(uint32_t)) {
            const int end = (i + CACHE_LINE / sizeof(uint32_t)) < npixels
                                ? i + (CACHE_LINE / sizeof(uint32_t))
                                : npixels;
            for (int j = i; j < end; j++) {
                const uint8_t *pix = &image->data[j * comps];
                const int dr = pix[R_OFFSET] - new_center[R_OFFSET];
                const int dg = pix[G_OFFSET] - new_center[G_OFFSET];
                const int db = pix[B_OFFSET] - new_center[B_OFFSET];
                const uint32_t dist = dr * dr + dg * dg + db * db;
                distances[j] = dist < distances[j] ? dist : distances[j];
            }
        }
        memcpy(&centers[c * comps], new_center, pixel_size);
    }
    free(distances);
}

void kmeans(struct img_t *image, int num_clusters) {
    const int comps = image->components;
    const int npixels = image->width * image->height;

    // Aligned allocations
    uint8_t *centers = alloc(CACHE_LINE, num_clusters * comps);
    int *assignments = alloc(CACHE_LINE, npixels * sizeof(int));
    ClusterData *clusters = alloc(CACHE_LINE, num_clusters * sizeof(ClusterData));

    kmeans_pp(image, num_clusters, centers);

    // Precompute squared center values
    for (int c = 0; c < num_clusters; c++) {
        const uint8_t *ctr = &centers[c * comps];
        clusters[c].sqsum = ctr[R_OFFSET] * ctr[R_OFFSET] + ctr[G_OFFSET] * ctr[G_OFFSET] + ctr[B_OFFSET] * ctr[B_OFFSET];
    }

    // Vectorized-style processing without SIMD
    for (int i = 0; i < npixels; i++) {
        const uint8_t *pix = &image->data[i * comps];
        int best_c = 0;
        int max_val = INT_MIN;

        // Process 4 clusters at a time
        for (int c = 0; c < num_clusters; c += UNROLL) {
            int vals[UNROLL];
            int max_unroll = (num_clusters - c) < UNROLL ? (num_clusters - c) : UNROLL;

            for (int u = 0; u < max_unroll; u++) {
                const uint8_t *ctr = &centers[(c + u) * comps];
                const int dot = pix[R_OFFSET] * ctr[R_OFFSET] + pix[G_OFFSET] * ctr[G_OFFSET] + pix[B_OFFSET] * ctr[B_OFFSET];
                vals[u] = 2 * dot - clusters[c + u].sqsum;
            }

            for (int u = 0; u < max_unroll; u++) {
                if (vals[u] > max_val) {
                    max_val = vals[u];
                    best_c = c + u;
                }
            }
        }
        assignments[i] = best_c;

        // Update cluster sums immediately
        clusters[best_c].count++;
        clusters[best_c].sum_r += pix[R_OFFSET];
        clusters[best_c].sum_g += pix[G_OFFSET];
        clusters[best_c].sum_b += pix[B_OFFSET];
    }

    // Update centers using fast reciprocal approximation
    for (int c = 0; c < num_clusters; c++) {
        if (clusters[c].count) {
            const float inv = 1.0f / clusters[c].count;
            centers[c * comps + R_OFFSET] = clusters[c].sum_r * inv + 0.5f;
            centers[c * comps + G_OFFSET] = clusters[c].sum_g * inv + 0.5f;
            centers[c * comps + B_OFFSET] = clusters[c].sum_b * inv + 0.5f;
        }
    }

    // Direct write with coalesced memory access
    for (int i = 0; i < npixels; i += CACHE_LINE / sizeof(uint8_t)) {
        const int end = (i + CACHE_LINE / sizeof(uint8_t)) < npixels
                            ? i + CACHE_LINE / sizeof(uint8_t)
                            : npixels;
        for (int j = i; j < end; j++) {
            const int c = assignments[j];
            uint8_t *pix = &image->data[j * comps];
            const uint8_t *ctr = &centers[c * comps];
            pix[R_OFFSET] = ctr[R_OFFSET];
            pix[G_OFFSET] = ctr[G_OFFSET];
            pix[B_OFFSET] = ctr[B_OFFSET];
        }
    }

    free(clusters);
    free(assignments);
    free(centers);
}

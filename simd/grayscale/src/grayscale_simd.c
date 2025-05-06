#include <immintrin.h>
#include <stdlib.h>

#include "grayscale.h"

void grayscale(struct img_t *image) {
    const int n_comp = image->components;
    const int n_px   = image->width * image->height;
    uint8_t  *src    = image->data;
    uint8_t  *dst;

    // Allocate an aligned 32 byte buffer for the output
    if (posix_memalign((void **)&dst, 32, n_px) != 0) {
        return;
    }

    int i = 0;
    // Prepare upscaled fixed-point weights
    const __m256i c_r   = _mm256_set1_epi32(299);  // weight for R component
    const __m256i c_g   = _mm256_set1_epi32(587);  // weight for G component
    const __m256i c_b   = _mm256_set1_epi32(114);  // weight for B component
    const __m256i c_add = _mm256_set1_epi32(512);  // rounding offset (0.5 * 2^10)

    // Process 8 pixels per loop iteration
    for (; i <= n_px - 8; i += 8) {
        __m256i chunk = _mm256_loadu_si256((__m256i *)(src + n_comp * i));
        // Split into two 128-bit lanes for byte-wise extraction
        __m128i pix_lo = _mm256_extracti128_si256(chunk, 0);
        __m128i pix_hi = _mm256_extracti128_si256(chunk, 1);

        __m256i p0      = _mm256_cvtepu8_epi32(pix_lo);                     // R
        __m256i p1      = _mm256_cvtepu8_epi32(_mm_srli_si128(pix_lo, 4));  // G
        __m256i p2      = _mm256_cvtepu8_epi32(_mm_srli_si128(pix_lo, 8));  // B
        __m256i gray_lo = _mm256_add_epi32(
            _mm256_add_epi32(
                _mm256_mullo_epi32(p0, c_r),
                _mm256_mullo_epi32(p1, c_g)),
            _mm256_mullo_epi32(p2, c_b));
        gray_lo = _mm256_add_epi32(gray_lo, c_add);
        gray_lo = _mm256_srli_epi32(gray_lo, 10);  // Downscale

        __m256i q0      = _mm256_cvtepu8_epi32(pix_hi);                     // R
        __m256i q1      = _mm256_cvtepu8_epi32(_mm_srli_si128(pix_hi, 4));  // G
        __m256i q2      = _mm256_cvtepu8_epi32(_mm_srli_si128(pix_hi, 8));  // B
        __m256i gray_hi = _mm256_add_epi32(
            _mm256_add_epi32(
                _mm256_mullo_epi32(q0, c_r),
                _mm256_mullo_epi32(q1, c_g)),
            _mm256_mullo_epi32(q2, c_b));  //
        gray_hi = _mm256_add_epi32(gray_hi, c_add);
        gray_hi = _mm256_srli_epi32(gray_hi, 10);  // Downscale

        // Unpack to 1 byte and store
        __m128i packed = _mm_packus_epi16(
            _mm256_castsi256_si128(_mm256_packs_epi32(gray_lo, gray_hi)),
            _mm256_extracti128_si256(_mm256_packs_epi32(gray_lo, gray_hi), 1));
        _mm_storel_epi64((__m128i *)(dst + i), packed);
    }

    // Handle remaining pixels
    for (; i < n_px; ++i) {
        uint8_t r = src[3 * i + R_OFFSET];
        uint8_t g = src[3 * i + G_OFFSET];
        uint8_t b = src[3 * i + B_OFFSET];
        dst[i]    = (uint8_t)((299 * r + 587 * g + 114 * b + 512) >> 10);  // With downscale
    }

    // Replace image data with grayscale buffer
    free(image->data);
    image->data       = dst;
    image->components = 1;
}

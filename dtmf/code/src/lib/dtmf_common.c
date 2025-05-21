/**
 * @file dtmf_common.c
 * @brief Common library functions.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include "dtmf_common.h"

#include <assert.h>
#include <dtmf_utils.h>
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>

#include "dtmf.h"
#include "dtmf_error.h"
#include "dtmf_utils.h"


DtmfFrequencyPair DTMF_FREQUENCIES_MAP[DTMF_NUM_TONES] = {
    {DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_1},
    {DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_2},
    {DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_3},
    {DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_1},
    {DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_2},
    {DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_3},
    {DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_1},
    {DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_2},
    {DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_3},
    {DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_1},
    {DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_2},
    {DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_3},
};

// FIXME: make tone sample a pointer to a tone vector
DtmfMapping dtmf_table_global[] = {
    {'1', 1,  1, {0, {0}}},
    {'2', 2,  1, {0, {0}}},
    {'A', 2,  2, {0, {0}}},
    {'B', 2,  3, {0, {0}}},
    {'C', 2,  4, {0, {0}}},
    {'3', 3,  1, {0, {0}}},
    {'D', 3,  2, {0, {0}}},
    {'E', 3,  3, {0, {0}}},
    {'F', 3,  4, {0, {0}}},
    {'4', 4,  1, {0, {0}}},
    {'G', 4,  2, {0, {0}}},
    {'H', 4,  3, {0, {0}}},
    {'I', 4,  4, {0, {0}}},
    {'5', 5,  1, {0, {0}}},
    {'J', 5,  2, {0, {0}}},
    {'K', 5,  3, {0, {0}}},
    {'L', 5,  4, {0, {0}}},
    {'6', 6,  1, {0, {0}}},
    {'M', 6,  2, {0, {0}}},
    {'N', 6,  3, {0, {0}}},
    {'O', 6,  4, {0, {0}}},
    {'7', 7,  1, {0, {0}}},
    {'P', 7,  2, {0, {0}}},
    {'Q', 7,  3, {0, {0}}},
    {'R', 7,  4, {0, {0}}},
    {'S', 7,  5, {0, {0}}},
    {'8', 8,  1, {0, {0}}},
    {'T', 8,  2, {0, {0}}},
    {'U', 8,  3, {0, {0}}},
    {'V', 8,  4, {0, {0}}},
    {'9', 9,  1, {0, {0}}},
    {'W', 9,  2, {0, {0}}},
    {'X', 9,  3, {0, {0}}},
    {'Y', 9,  4, {0, {0}}},
    {'Z', 9,  5, {0, {0}}},
    {'#', 10, 1, {0, {0}}},
    {'.', 10, 2, {0, {0}}},
    {'!', 10, 3, {0, {0}}},
    {'?', 10, 4, {0, {0}}},
    {',', 10, 5, {0, {0}}},
    {'0', 11, 1, {0, {0}}},
    {' ', 11, 2, {0, {0}}},
    {'*', 12, 1, {0, {0}}},
    {'*', 12, 2, {0, {0}}},
    {'*', 12, 3, {0, {0}}},
    {'*', 12, 4, {0, {0}}},
    {'*', 12, 5, {0, {0}}},
};

int dtmf_table_size = sizeof(dtmf_table_global) / sizeof(dtmf_table_global[0]);

DtmfTone dtmf_tones_map_global[DTMF_NUM_TONES];

static bool dtmf_initialized       = false;
static bool dtmf_table_initialized = false;
static bool dtmf_tones_initialized = false;

void _dtmf_gen_tone(DtmfFrequencyPair const frequencies, dtmf_float_t *buffer) {
    assert(buffer != NULL);

    LIKWID_MARKER_START("encode-tone-gen");

    for (size_t n = 0; n < DTMF_TONE_NUM_SAMPLES; n++) {
        dtmf_float_t t = (dtmf_float_t)n / (dtmf_float_t)DTMF_SAMPLE_RATE_HZ;

        buffer[n] = (DTMF_AMPLITUDE) * (sin(2 * M_PI * frequencies.low * t) +
                                        sin(2 * M_PI * frequencies.high * t));
    }

    LIKWID_MARKER_STOP("encode-tone-gen");
}

void _dtmf_init_tones_map() {
    if (dtmf_tones_initialized) {
        return;
    }

    // Pass the frequency pairs from frequency map to the generator function.
    for (int i = 0; i < DTMF_NUM_TONES; i++) {
        _dtmf_gen_tone(DTMF_FREQUENCIES_MAP[i], dtmf_tones_map_global[i].samples);
    }

    dtmf_tones_initialized = true;
    debug_printf("Tones map initialized");
}

void _dtmf_init_table() {
    if (dtmf_table_initialized) {
        return;
    }

    for (int i = 0; i < dtmf_table_size; i++) {
        DtmfMapping *mapping = &dtmf_table_global[i];

        dtmf_count_t  buffer_write_ptr = 0;
        dtmf_float_t *buffer_src       = dtmf_tones_map_global[mapping->key - 1].samples;
        dtmf_float_t *buffer_dest      = mapping->tone.samples;

        assert(buffer_dest != NULL);

        for (int j = 0; j < mapping->presses; j++) {
            // Copy the tone samples.
            memcpy(buffer_dest + buffer_write_ptr, buffer_src, DTMF_TONE_NUM_SAMPLES * sizeof(dtmf_float_t));
            buffer_write_ptr += DTMF_TONE_NUM_SAMPLES;

            // Add silence between tone repeats.
            if (mapping->presses > 1 && j < mapping->presses - 1) {
                memset(buffer_dest + buffer_write_ptr, 0, DTMF_TONE_REPEAT_NUM_SAMPLES * sizeof(dtmf_float_t));
                buffer_write_ptr += DTMF_TONE_REPEAT_NUM_SAMPLES;
            }
        }

        mapping->tone.sample_count = buffer_write_ptr;
    }

    dtmf_table_initialized = true;
    debug_printf("Mappings table initialized");
}

void _dtmf_init() {
    if (dtmf_initialized) {
        return;
    }

    _dtmf_init_tones_map();
    _dtmf_init_table();

    dtmf_initialized = true;

    debug_printf("DTMF module initialized");
}

bool _dtmf_allocate_buffer(dtmf_float_t **buffer, size_t num_samples) {
    DTMF_TRACE("Allocating buffer of size %lu\n", num_samples);
    assert(buffer != NULL);

    *buffer = (dtmf_float_t *)malloc(num_samples * sizeof(dtmf_float_t));

    if (*buffer == NULL) {
        DTMF_ERROR("Could not allocate buffer of size %lu", num_samples);
    }

    DTMF_TRACE("Allocated buffer of size %lu x %lu bytes", num_samples, sizeof(dtmf_float_t));

    DTMF_SUCCEED();
}

bool _dtmf_free_buffer(dtmf_float_t *buffer) {
    assert(buffer != NULL);

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
        return true;
    }

    return false;
}

char _dtmf_map_presses_to_letter(dtmf_count_t key, dtmf_count_t presses) {
    for (int i = 0; i < dtmf_table_size; i++) {
        if (dtmf_table_global[i].key == key && dtmf_table_global[i].presses == presses) {
            return dtmf_table_global[i].letter;
        }
    }

    debug_printf("No matching letter for key %lu with %lu presses", key, presses);
    return DTMF_UNKNOWN_SYMBOL;
}

static dtmf_float_t _dtmf_compute_rms(const dtmf_float_t *buffer, dtmf_count_t const count) {
    assert(buffer != NULL);

    dtmf_float_t sum_squares = 0.0;

    for (size_t i = 0; i < count; i++) {
        sum_squares += buffer[i] * buffer[i];
    }

    return sqrt(sum_squares / (dtmf_float_t)count);
}

static void _dtmf_noise_reduction(dtmf_float_t *buffer,
                                  dtmf_count_t  count,
                                  dtmf_float_t  threshold_factor) {
    assert(buffer != NULL);

    dtmf_float_t rms       = _dtmf_compute_rms(buffer, count);
    dtmf_float_t threshold = rms * threshold_factor;

    const __m256d abs_mask = _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFFULL));
    const __m256d thr_v    = _mm256_set1_pd(threshold);
    size_t        i        = 0;
    // Vectorized loop 4 samples at a time
    for (; i + 3 < count; i += 4) {
        __m256d v     = _mm256_loadu_pd(&buffer[i]);              // Load
        __m256d abs_v = _mm256_and_pd(v, abs_mask);               // |v| = v & abs_mask
        __m256d m     = _mm256_cmp_pd(abs_v, thr_v, _CMP_LT_OS);  // mask = (abs_v < threshold)
        __m256d zero  = _mm256_setzero_pd();
        __m256d res   = _mm256_blendv_pd(v, zero, m);  // blend to 0 or keep v
        _mm256_storeu_pd(&buffer[i], res);             // store
    }

    for (; i < count; ++i) {
        if (fabs(buffer[i]) < threshold) {
            buffer[i] = 0.0;
        }
    }

    debug_printf("Noise reduction applied with dynamic threshold %f (RMS: %f, factor: %f)",
                 threshold, rms, threshold_factor);
}

static void _dtmf_normalize_signal(dtmf_float_t *buffer, dtmf_count_t const count) {
    dtmf_float_t max = 0.0;
    // Find maximum absolute value
    for (dtmf_count_t i = 0; i < count; i++) {
        if (fabs(buffer[i]) > max)
            max = fabs(buffer[i]);
    }
    // Normalize if max is not zero
    if (max > 0.0001) {
        for (dtmf_count_t i = 0; i < count; i++) {
            buffer[i] /= max;
        }
    }
}

static void _dtmf_apply_bandpass(dtmf_float_t *buffer, dtmf_count_t const count) {
    // IIR bandpass filter coefficients for 44100 Hz sample rate with some tolerance
    const dtmf_float_t b0 = 0.0032981, b1 = 0.0, b2 = -0.00659619, b3 = 0.0, b4 = 0.0032981;
    const dtmf_float_t a1 = -3.79262674, a2 = 5.43304806, a3 = -3.48434686, a4 = 0.84429627;
    dtmf_float_t       x1 = 0, x2 = 0, x3 = 0, x4 = 0, y1 = 0, y2 = 0, y3 = 0, y4 = 0, y;

    for (dtmf_count_t i = 0; i < count; i++) {
        y         = b0 * buffer[i] + b1 * x1 + b2 * x2 + b3 * x3 + b4 * x4 - a1 * y1 - a2 * y2 - a3 * y3 - a4 * y4;
        x4        = x3;
        x3        = x2;
        x2        = x1;
        x1        = buffer[i];
        y4        = y3;
        y3        = y2;
        y2        = y1;
        y1        = y;
        buffer[i] = y;
    }
}

// FIR + SIMD variant
// static void _dtmf_apply_bandpass(dtmf_float_t *buffer, dtmf_count_t const count) {
//     // FIR filter parameters
//     const int          fir_order     = 7;
//     const dtmf_float_t fir_coeffs[7] = {0.0220102, 0.0921197, 0.2392328, 0.3152551, 0.2392328, 0.0921197, 0.0220102};
//
//     // Circular buffer for previous samples
//     static dtmf_float_t state[7]  = {0};
//     static int          state_idx = 0;
//
//     // Load symmetric coefficient pairs into AVX registers
//     const __m256d coeffs_pairs = _mm256_setr_pd(
//         fir_coeffs[0],
//         fir_coeffs[1],
//         fir_coeffs[2],
//         0.0);
//
//     // Process each sample in the buffer
//     for (dtmf_count_t i = 0; i < count; i++) {
//         // Update circular buffer with new sample
//         state[state_idx]      = buffer[i];
//         const int current_idx = state_idx;
//         state_idx             = (state_idx + 1) % fir_order;
//
//         // Compute indices for symmetric taps on the buffer
//         const int idx[7] = {
//             current_idx,
//             (current_idx - 1 + 7) % 7,
//             (current_idx - 2 + 7) % 7,
//             (current_idx - 3 + 7) % 7,
//             (current_idx - 4 + 7) % 7,
//             (current_idx - 5 + 7) % 7,
//             (current_idx - 6 + 7) % 7};
//
//         // Load symmetric state pairs into AVX register
//         __m256d states_pair = _mm256_setr_pd(
//             state[idx[0]] + state[idx[6]],
//             state[idx[1]] + state[idx[5]],
//             state[idx[2]] + state[idx[4]],
//             0.0  // Padding
//         );
//
//         // Vectorized multiply-add operation
//         __m256d partial_sum = _mm256_mul_pd(states_pair, coeffs_pairs);
//
//         // Sum of vector elements
//         dtmf_float_t sum = ((dtmf_float_t *)&partial_sum)[0] + ((dtmf_float_t *)&partial_sum)[1] + ((dtmf_float_t *)&partial_sum)[2];
//
//         // Central term contribution
//         sum += state[idx[3]] * fir_coeffs[3];
//
//         // Store filtered value
//         buffer[i] = sum;
//     }
// }

static dtmf_float_t _dtmf_calculate_noise_threshold(dtmf_float_t const *buffer, dtmf_count_t const count, dtmf_float_t const threshold_factor) {
    dtmf_float_t sum    = 0.0;
    dtmf_float_t sum_sq = 0.0;
    for (dtmf_count_t i = 0; i < count; i++) {
        sum += buffer[i];
        sum_sq += buffer[i] * buffer[i];
    }
    dtmf_float_t mean     = sum / (dtmf_float_t)count;
    dtmf_float_t variance = (sum_sq / (dtmf_float_t)count) - (mean * mean);
    dtmf_float_t stddev   = sqrt(variance);
    return mean + threshold_factor * stddev;
}

static void _dtmf_pre_emphasis(dtmf_float_t *buffer, dtmf_count_t const count) {
    const dtmf_float_t alpha = 0.95;
    dtmf_float_t       prev  = 0.0;

    for (dtmf_count_t i = 0; i < count; i++) {
        dtmf_float_t current = buffer[i];
        buffer[i]            = buffer[i] - alpha * prev;
        prev                 = current;
    }
}

void _dtmf_preprocess_buffer(dtmf_float_t *buffer, dtmf_count_t frame_count, dtmf_float_t const threshold_factor) {
    LIKWID_MARKER_START("decode-preprocess");

    dtmf_float_t const noise_threshold = _dtmf_calculate_noise_threshold(buffer, frame_count, threshold_factor);
    _dtmf_noise_reduction(buffer, frame_count, noise_threshold);
    _dtmf_normalize_signal(buffer, frame_count);
    _dtmf_apply_bandpass(buffer, frame_count);
    _dtmf_pre_emphasis(buffer, frame_count);

    LIKWID_MARKER_STOP("decode-preprocess");
}

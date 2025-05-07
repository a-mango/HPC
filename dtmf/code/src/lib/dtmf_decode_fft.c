/**
 * @file dtmf_decode_fft.c
 * @brief DTMF decoding functions implementation using FFT algorithm.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-04-02
 */

#include <assert.h>
#include <fftw3.h>
#include <immintrin.h>
#include <likwid-marker.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "dtmf.h"
#include "dtmf_common.h"
#include "dtmf_error.h"
#include "dtmf_utils.h"

#define DTMF_FFT_SIZE 2048
#define DTMF_MIN_PAUSE_BEFORE_NEW_TONE 2
#define DTMF_DETECT_KEY_INVALID -1
#define DTMF_FFT_NOISE_THRESHOLD 70
#define DTMF_PREPROCESS_THRESHOLD_FACTOR 1.1

// One‐time buffers aligned for AVX2
static dtmf_float_t buff_real[DTMF_FFT_SIZE] __attribute__((aligned(32)));
static dtmf_float_t hanning_window[DTMF_FFT_SIZE] __attribute__((aligned(32)));
static bool         window_initialized = false;

// Build the Hanning window table once
static void init_hanning_window(void) {
    for (int i = 0; i < DTMF_FFT_SIZE; ++i) {
        // original uses M_2_PI == 2/π
        hanning_window[i] = (dtmf_float_t)0.5 * (1.0 - cos(M_2_PI * i / (DTMF_FFT_SIZE - 1)));
    }
    window_initialized = true;
}

static int fft_detect(dtmf_float_t const *samples,
                      dtmf_count_t        num_samples,
                      dtmf_float_t        sample_rate) {
    assert(samples != NULL);
    if (!window_initialized) {
        init_hanning_window();
    }

    // Allocate FFTW buffers
    fftw_complex *in  = fftw_malloc(sizeof(*in) * DTMF_FFT_SIZE);
    fftw_complex *out = fftw_malloc(sizeof(*out) * DTMF_FFT_SIZE);
    fftw_plan     p   = fftw_plan_dft_1d(DTMF_FFT_SIZE, in, out,
                                         FFTW_FORWARD, FFTW_ESTIMATE);

    int          detected_key  = -1;
    dtmf_float_t max_magnitude = 0;

    // Apply Hanning window with AVX2
    LIKWID_MARKER_START("decode-fft-hanning");
    {
        // Sample count
        size_t valid = (num_samples < DTMF_FFT_SIZE)
                           ? (size_t)num_samples
                           : (size_t)DTMF_FFT_SIZE;
        size_t i     = 0;

        // Vectorized multiply of 4 doubles at a time
        for (; i + 3 < valid; i += 4) {
            __m256d samp = _mm256_loadu_pd(&samples[i]);
            __m256d wind = _mm256_load_pd(&hanning_window[i]);
            __m256d prod = _mm256_mul_pd(samp, wind);
            _mm256_store_pd(&buff_real[i], prod);
        }

        // Remainder of valid region
        for (; i < valid; ++i) {
            buff_real[i] = samples[i] * hanning_window[i];
        }

        // Zero‐pad remainder up to FFT size
        for (; i < DTMF_FFT_SIZE; ++i) {
            buff_real[i] = 0.0;
        }

        // Pack into FFTW complex array
        for (int idx = 0; idx < DTMF_FFT_SIZE; ++idx) {
            in[idx][0] = buff_real[idx];
            in[idx][1] = 0.0;
        }
    }
    LIKWID_MARKER_STOP("decode-fft-hanning");

    // Execute FFT
    fftw_execute(p);

    // Frequency‐bin magnitude detection
    LIKWID_MARKER_START("decode-fft-magnitude");
    for (int key = 0; key < DTMF_NUM_TONES; ++key) {
        dtmf_float_t low_freq  = DTMF_FREQUENCIES_MAP[key].low;
        dtmf_float_t high_freq = DTMF_FREQUENCIES_MAP[key].high;
        int          low_idx   = (int)(low_freq * DTMF_FFT_SIZE / sample_rate);
        int          high_idx  = (int)(high_freq * DTMF_FFT_SIZE / sample_rate);

        dtmf_float_t magnitude = 0.0;
        for (int d = -1; d <= 1; ++d) {
            int li = low_idx + d;
            int hi = high_idx + d;
            if (li >= 0 && li < DTMF_FFT_SIZE)
                magnitude += hypot(out[li][0], out[li][1]);
            if (hi >= 0 && hi < DTMF_FFT_SIZE)
                magnitude += hypot(out[hi][0], out[hi][1]);
        }

        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            detected_key  = key + 1;
        }
    }
    LIKWID_MARKER_STOP("decode-fft-magnitude");

    // Clean up
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    // Noise threshold check
    if (max_magnitude < DTMF_FFT_NOISE_THRESHOLD) {
        return DTMF_DETECT_KEY_INVALID;
    }
    return detected_key;
}

bool dtmf_decode(dtmf_float_t *dtmf_buffer,
                 dtmf_count_t  frame_count,
                 char        **out_message,
                 dtmf_count_t *out_chars_read) {
    assert(dtmf_buffer != NULL);
    assert(frame_count > 0);
    assert(out_message != NULL);
    assert(out_chars_read != NULL);

    DTMF_DEBUG("Decoding DTMF signal using FFT...");
    LIKWID_MARKER_START("decode-fft");

    // Preprocess
    _dtmf_preprocess_buffer(dtmf_buffer,
                            frame_count,
                            DTMF_PREPROCESS_THRESHOLD_FACTOR);

    dtmf_count_t buffer_read_ptr   = 0;
    dtmf_count_t chunk_size        = DTMF_TONE_REPEAT_NUM_SAMPLES;
    dtmf_count_t message_length    = 0;
    int          last_detected_key = -1;
    dtmf_count_t chunks_seen       = 0;
    dtmf_count_t repetitions       = 0;
    dtmf_count_t pause_repetitions = 0;
    dtmf_count_t max_message_length =
        frame_count / (DTMF_TONE_NUM_SAMPLES + DTMF_TONE_PAUSE_NUM_SAMPLES) + 1;

    *out_message = calloc(max_message_length + 1, sizeof(char));
    if (*out_message == NULL) {
        fprintf(stderr,
                "Error: Could not allocate memory for output message\n");
        LIKWID_MARKER_STOP("decode-fft");
        return false;
    }

    LIKWID_MARKER_START("decode-fft-mainloop");
    while (buffer_read_ptr < frame_count) {
        int detected_key = fft_detect(dtmf_buffer + buffer_read_ptr,
                                      (frame_count - buffer_read_ptr < chunk_size)
                                          ? frame_count - buffer_read_ptr
                                          : chunk_size,
                                      DTMF_SAMPLE_RATE_HZ);

        if (detected_key != DTMF_DETECT_KEY_INVALID) {
            if (pause_repetitions < DTMF_MIN_PAUSE_BEFORE_NEW_TONE &&
                last_detected_key != -1 &&
                detected_key != last_detected_key) {
                // Reject fast switch between tones
                detected_key = last_detected_key;
            }

            if (last_detected_key == -1) {
                last_detected_key = detected_key;
                chunks_seen       = 1;
            } else if (last_detected_key == detected_key) {
                chunks_seen++;
            } else {
                last_detected_key = detected_key;
                chunks_seen       = 1;
            }
            pause_repetitions = 0;
        } else {
            pause_repetitions++;
            if (chunks_seen >= 2) {
                repetitions++;
                chunks_seen = 0;
            }
            if (pause_repetitions >= 3 && last_detected_key != -1) {
                char letter =
                    _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key,
                                                repetitions);
                debug_printf("Detected letter %c", letter);
                if (message_length < max_message_length) {
                    (*out_message)[message_length++] = letter;
                }
                pause_repetitions = 0;
                repetitions       = 0;
                chunks_seen       = 0;
                last_detected_key = -1;
            }
        }

        // Final‐chunk edge‐case
        if (buffer_read_ptr + chunk_size >= frame_count &&
            last_detected_key != -1) {
            if (chunks_seen >= 2)
                repetitions++;
            char letter =
                _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key,
                                            repetitions);
            debug_printf("Detected letter %c", letter);
            if (message_length < max_message_length) {
                (*out_message)[message_length++] = letter;
            }
        }

        buffer_read_ptr += chunk_size;
    }
    LIKWID_MARKER_STOP("decode-fft-mainloop");

    *out_chars_read = message_length;
    LIKWID_MARKER_STOP("decode-fft");

    DTMF_DEBUG("Message successfully decoded.");
    DTMF_SUCCEED();
    return true;
}

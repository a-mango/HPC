/**
 * @file dtmf_decode_fft.c
 * @brief DTMF decoding functions implementation using FFT algorithm.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dtmf.h"
#include "dtmf_common.h"
#include "dtmf_error.h"
#include "dtmf_utils.h"


#define FFT_SIZE 2048
#define MIN_PAUSE_BEFORE_NEW_TONE 2

// Magic numbers
#define FFT_NOISE_THRESHOLD 70
#define PREPROCESS_THRESHOLD_FACTOR 1.1

static int fft_detect(dtmf_float_t const *samples, dtmf_count_t num_samples, dtmf_float_t sample_rate) {
    fftw_complex *in, *out;
    fftw_plan     p;
    int           detected_key  = -1;
    dtmf_float_t  max_magnitude = 0;

    in  = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    p   = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Hanning window
    for (int i = 0; i < FFT_SIZE; i++) {
        dtmf_float_t window = 0.5 * (1 - cos(M_2_PI * i / (FFT_SIZE - 1)));
        in[i][0]            = ((dtmf_count_t)i < num_samples) ? samples[i] * window : 0.0;
        in[i][1]            = 0.0;
    }

    fftw_execute(p);

    for (int key = 0; key < DTMF_NUM_TONES; key++) {
        dtmf_float_t low_freq   = DTMF_FREQUENCIES_MAP[key].low;
        dtmf_float_t high_freq  = DTMF_FREQUENCIES_MAP[key].high;
        int          low_index  = (int)(low_freq * FFT_SIZE / sample_rate);
        int          high_index = (int)(high_freq * FFT_SIZE / sample_rate);

        dtmf_float_t magnitude = 0.0;

        for (int delta = -1; delta <= 1; delta++) {
            int li = low_index + delta;
            int hi = high_index + delta;

            if (li >= 0 && li < FFT_SIZE)
                magnitude += hypot(out[li][0], out[li][1]);
            if (hi >= 0 && hi < FFT_SIZE)
                magnitude += hypot(out[hi][0], out[hi][1]);
        }

        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            detected_key  = key + 1;
        }
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    if (max_magnitude < FFT_NOISE_THRESHOLD) {
        return -1;
    }

    return detected_key;
}

bool dtmf_decode(dtmf_float_t *dtmf_buffer, dtmf_count_t const frame_count, char **out_message, dtmf_count_t *out_chars_read) {
    assert(dtmf_buffer != NULL);

    _dtmf_preprocess_buffer(dtmf_buffer, frame_count, PREPROCESS_THRESHOLD_FACTOR);

    dtmf_count_t buffer_read_ptr = 0;
    dtmf_count_t chunk_size      = DTMF_TONE_REPEAT_NUM_SAMPLES;

    dtmf_count_t message_length     = 0;
    int          last_detected_key  = -1;
    dtmf_count_t chunks_seen        = 0;
    dtmf_count_t repetitions        = 0;
    dtmf_count_t pause_repetitions  = 0;
    dtmf_count_t max_message_length = frame_count / (DTMF_TONE_NUM_SAMPLES + DTMF_TONE_PAUSE_NUM_SAMPLES) + 1;

    *out_message = (char *)calloc(max_message_length + 1, sizeof(char));
    if (*out_message == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for output message\n");
        return 0;
    }

    while (buffer_read_ptr < frame_count) {
        int detected_key = fft_detect(dtmf_buffer + buffer_read_ptr, chunk_size, DTMF_SAMPLE_RATE_HZ);

        if (detected_key != -1) {
            if (pause_repetitions < MIN_PAUSE_BEFORE_NEW_TONE && last_detected_key != -1 && detected_key != last_detected_key) {
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
                char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
                debug_printf("Detected letter %c\n", letter);
                if (message_length < max_message_length) {
                    (*out_message)[message_length++] = letter;
                }

                pause_repetitions = 0;
                repetitions       = 0;
                chunks_seen       = 0;
                last_detected_key = -1;
            }
        }

        // Final chunk edge-case handling
        if (buffer_read_ptr + chunk_size >= frame_count && last_detected_key != -1) {
            if (chunks_seen >= 2) {
                repetitions++;
            }

            char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
            debug_printf("Detected letter %c\n", letter);
            if (message_length < max_message_length) {
                (*out_message)[message_length++] = letter;
            }
        }

        buffer_read_ptr += chunk_size;
    }

    *out_chars_read = message_length;

    DTMF_SUCCEED();
}

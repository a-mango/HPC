/**
 * @file dtmf_decode_goertzel.c
 * @brief DTMF decoding functions implementation using FFT algorithm.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <fftw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dtmf.h"
#include "dtmf_common.h"
#include "io_utils.h"

#define FFT_SIZE 2048

static int fft_detect(dtmf_float_t const *samples, dtmf_count_t num_samples, dtmf_float_t sample_rate) {
    fftw_complex *in, *out;
    fftw_plan     p;
    int           detected_key  = -1;
    dtmf_float_t  max_magnitude = 0;

    in  = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    p   = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (dtmf_count_t i = 0; i < FFT_SIZE; i++) {
        in[i][0] = (i < num_samples) ? samples[i] : 0.0;
        in[i][1] = 0.0;
    }

    fftw_execute(p);

    for (int key = 0; key <= DTMF_NUM_TONES - 1; key++) {
        dtmf_float_t low_freq   = DTMF_FREQUENCIES_MAP[key].low;
        dtmf_float_t high_freq  = DTMF_FREQUENCIES_MAP[key].high;
        int          low_index  = (int)(low_freq * FFT_SIZE / sample_rate);
        int          high_index = (int)(high_freq * FFT_SIZE / sample_rate);

        dtmf_float_t magnitude = sqrt(out[low_index][0] * out[low_index][0] + out[low_index][1] * out[low_index][1]) +
                                 sqrt(out[high_index][0] * out[high_index][0] + out[high_index][1] * out[high_index][1]);

        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            detected_key  = key + 1;
        }
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return detected_key;
}

dtmf_count_t dtmf_decode(dtmf_float_t const *dtmf_buffer, char **out_message, dtmf_count_t dtmf_frame_count) {
    assert(dtmf_buffer != NULL);

    dtmf_count_t buffer_read_ptr = 0;
    dtmf_count_t chunk_size      = DTMF_TONE_REPEAT_NUM_SAMPLES;

    dtmf_count_t message_length     = 0;
    int          last_detected_key  = -1;
    dtmf_count_t chunks_seen        = 0;
    dtmf_count_t repetitions        = 0;
    dtmf_count_t pause_repetitions  = 0;
    dtmf_count_t max_message_length = dtmf_frame_count / (DTMF_TONE_NUM_SAMPLES + DTMF_TONE_PAUSE_NUM_SAMPLES) + 1;

    *out_message = (char *)calloc(max_message_length, sizeof(char));
    if (*out_message == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for output message\n");
        return 0;
    }

    while (buffer_read_ptr < dtmf_frame_count) {
        int detected_key = fft_detect(dtmf_buffer + buffer_read_ptr, chunk_size, DTMF_SAMPLE_RATE_HZ);

        if (detected_key != -1) {
            if (last_detected_key == -1) {
                last_detected_key = detected_key;
            }

            if (last_detected_key == detected_key) {
                chunks_seen++;
            } else {
                last_detected_key = detected_key;
                chunks_seen       = 1;
            }

            pause_repetitions = 0;
        } else {
            pause_repetitions++;

            if (chunks_seen >= 4) {
                repetitions++;
                chunks_seen = 0;
            }

            if (pause_repetitions >= 4) {
                char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
                debug_printf("Detected letter %c\n", letter);
                (*out_message)[message_length++] = letter;
                pause_repetitions                = 0;
                repetitions                      = 0;
                chunks_seen                      = 0;
                last_detected_key                = -1;
            }
        }

        // Special case for the last chunk
        if (buffer_read_ptr + chunk_size >= dtmf_frame_count) {
            if (chunks_seen >= 4) {
                repetitions++;
                chunks_seen = 0;
            }

            if (last_detected_key != -1) {
                char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
                debug_printf("Detected letter %c\n", letter);
                (*out_message)[message_length++] = letter;
            }
        }

        buffer_read_ptr += chunk_size;
    }

    return message_length;
}

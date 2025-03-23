/**
 * @file dtmf_decode_goertzel.c
 * @brief DTMF decoding functions implementation using Goertzel algorithm.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dtmf.h"
#include "dtmf_common.h"
#include "io_utils.h"

#define NOISE_THRESHOLD 15
#define SILENCE_THRESHOLD 30

static void apply_hamming_window(dtmf_float_t *samples, dtmf_count_t num_samples) {
    for (dtmf_count_t i = 0; i < num_samples; i++) {
        samples[i] *= 0.54 - 0.46 * cos((2 * M_PI * i) / (num_samples - 1));
    }
}

static dtmf_float_t goertzel_detect(dtmf_float_t const *samples, dtmf_count_t num_samples, dtmf_float_t target_freq, dtmf_float_t sample_rate) {
    int          k      = (int)(0.5 + (((dtmf_float_t)num_samples * target_freq) / sample_rate));
    dtmf_float_t omega  = (2.0 * M_PI * k) / (dtmf_float_t)num_samples;
    dtmf_float_t sine   = sin(omega);
    dtmf_float_t cosine = cos(omega);
    dtmf_float_t coeff  = 2.0 * cosine;
    dtmf_float_t q0 = 0, q1 = 0, q2 = 0;

    // Create a copy of the samples to apply the Hamming window
    dtmf_float_t *windowed_samples = (dtmf_float_t *)malloc(num_samples * sizeof(dtmf_float_t));
    if (windowed_samples == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for windowed samples\n");
        return 0;
    }
    memcpy(windowed_samples, samples, num_samples * sizeof(dtmf_float_t));
    apply_hamming_window(windowed_samples, num_samples);

    for (dtmf_count_t i = 0; i < num_samples; i++) {
        q0 = coeff * q1 - q2 + windowed_samples[i];
        q2 = q1;
        q1 = q0;
    }

    free(windowed_samples);

    dtmf_float_t real      = (q1 - q2 * cosine);
    dtmf_float_t imag      = (q2 * sine);
    dtmf_float_t magnitude = sqrt(real * real + imag * imag);

    return magnitude;
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
        dtmf_float_t max_magnitude = 0;
        int          detected_key  = -1;

        // Detect the key for the current 50ms chunk
        for (int key = 1; key <= 11; key++) {
            dtmf_float_t magnitude = goertzel_detect(dtmf_buffer + buffer_read_ptr, chunk_size, DTMF_FREQUENCIES_MAP[key - 1].low, DTMF_SAMPLE_RATE_HZ);
            magnitude += goertzel_detect(dtmf_buffer + buffer_read_ptr, chunk_size, DTMF_FREQUENCIES_MAP[key - 1].high, DTMF_SAMPLE_RATE_HZ);

            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
                detected_key  = key;
            }
        }

        debug_printf("Detected key %d with magnitude %f\n", detected_key, max_magnitude);
        if (max_magnitude < NOISE_THRESHOLD) {
            detected_key = -1;
        }

        if (detected_key != -1) {
            if (last_detected_key == -1) {
                last_detected_key = detected_key;
            }

            if (last_detected_key == detected_key) {
                chunks_seen++;
            } else {
                chunks_seen = 0;
            }

            pause_repetitions = 0;
        } else {
            pause_repetitions++;

            if (chunks_seen == 4) {
                repetitions++;
                chunks_seen = 0;
            }

            if (pause_repetitions == 4) {
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
            if (chunks_seen == 4) {
                repetitions++;
                chunks_seen = 0;
            }

            (*out_message)[message_length++] = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
        }

        buffer_read_ptr += chunk_size;
    }

    // Apply noise gate to the output message
    for (dtmf_count_t i = 0; i < message_length; i++) {
        if ((*out_message)[i] < SILENCE_THRESHOLD) {
            (*out_message)[i] = ' ';
        }
    }

    return message_length;
}

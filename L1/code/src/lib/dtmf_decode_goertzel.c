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

#define NOISE_THRESHOLD 43
#define SILENCE_THRESHOLD 27
#define DTMF_COMPONENT_RATIO_THRESHOLD 0.4

static dtmf_float_t goertzel_detect(dtmf_float_t const *samples, dtmf_count_t num_samples, dtmf_float_t target_freq, dtmf_float_t sample_rate) {
    int          k      = (int)(0.5 + (((dtmf_float_t)num_samples * target_freq) / sample_rate));
    dtmf_float_t omega  = (2.0 * M_PI * k) / (dtmf_float_t)num_samples;
    dtmf_float_t sine   = sin(omega);
    dtmf_float_t cosine = cos(omega);
    dtmf_float_t coeff  = 2.0 * cosine;
    dtmf_float_t q0 = 0, q1 = 0, q2 = 0;

    for (dtmf_count_t i = 0; i < num_samples; i++) {
        dtmf_float_t windowed_sample = samples[i] * (0.54 - 0.46 * cos((2 * M_PI * (dtmf_float_t)i) / (dtmf_float_t)(num_samples - 1)));
        q0 = coeff * q1 - q2 + windowed_sample;
        q2 = q1;
        q1 = q0;
    }

    dtmf_float_t real      = (q1 - q2 * cosine);
    dtmf_float_t imag      = (q2 * sine);
    dtmf_float_t magnitude = sqrt(real * real + imag * imag);

    return magnitude;
}

static bool is_dtmf_tone_detected(dtmf_float_t low_mag, dtmf_float_t high_mag) {
    if (low_mag < NOISE_THRESHOLD || high_mag < NOISE_THRESHOLD) return false;
    dtmf_float_t ratio = low_mag < high_mag ? (low_mag / high_mag) : (high_mag / low_mag);
    return ratio > DTMF_COMPONENT_RATIO_THRESHOLD;
}

dtmf_count_t dtmf_decode(dtmf_float_t *dtmf_buffer, char **out_message, dtmf_count_t dtmf_frame_count) {
    assert(dtmf_buffer != NULL);

    dtmf_float_t noise_threshold = _dtmf_calculate_noise_threshold(dtmf_buffer, dtmf_frame_count);
    _dtmf_noise_reduction(dtmf_buffer, dtmf_frame_count, noise_threshold);
    _dtmf_normalize_signal(dtmf_buffer, dtmf_frame_count);
    _dtmf_apply_bandpass(dtmf_buffer, dtmf_frame_count);
    _dtmf_pre_emphasis(dtmf_buffer, dtmf_frame_count);

    const dtmf_count_t window_size = DTMF_TONE_REPEAT_NUM_SAMPLES;
    const dtmf_count_t stride_size = DTMF_TONE_REPEAT_NUM_SAMPLES / 2;

    dtmf_count_t message_length     = 0;
    dtmf_count_t max_message_length = dtmf_frame_count / stride_size + 1;
    *out_message                    = (char *)calloc(max_message_length, sizeof(char));
    if (!*out_message) {
        fprintf(stderr, "Error: Could not allocate memory for output message\n");
        return 0;
    }

    int          last_detected_key = -1;
    dtmf_count_t chunks_seen       = 0;
    dtmf_count_t repetitions       = 0;
    dtmf_count_t pause_repetitions = 0;
    bool message_started = false;
bool first_letter_decoded = false;

    dtmf_count_t debounce_window = DTMF_TONE_NUM_SAMPLES / stride_size;
    dtmf_count_t key_cooldown    = 0;

    for (dtmf_count_t i = 0; i + window_size <= dtmf_frame_count; i += stride_size) {
        dtmf_float_t max_magnitude = 0;
        int detected_key = -1;
        dtmf_float_t low_mag = 0, high_mag = 0;

        for (int key = 1; key <= 11; key++) {
            dtmf_float_t low = goertzel_detect(dtmf_buffer + i, window_size, DTMF_FREQUENCIES_MAP[key - 1].low, DTMF_SAMPLE_RATE_HZ);
            dtmf_float_t high = goertzel_detect(dtmf_buffer + i, window_size, DTMF_FREQUENCIES_MAP[key - 1].high, DTMF_SAMPLE_RATE_HZ);
            dtmf_float_t magnitude = low + high;

            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
                detected_key  = key;
                low_mag = low;
                high_mag = high;
            }
        }

        if (max_magnitude < NOISE_THRESHOLD) {
            detected_key = -1;
        }

        debug_printf("Window at %d detected key %d (mag=%.2f)\n", i, detected_key, max_magnitude);

        if (!message_started) {
    if (detected_key == -1) {
        continue; // wait until we detect a strong DTMF tone
    }
    message_started = true;
}

        if (detected_key != -1) {
            if (last_detected_key == -1) {
                last_detected_key = detected_key;
                chunks_seen       = 1;
            } else if (last_detected_key == detected_key) {
                chunks_seen++;
            } else {
                chunks_seen       = 1;
                repetitions       = 0;
                last_detected_key = detected_key;
            }
            pause_repetitions = 0;
        } else {
            pause_repetitions++;

            if (chunks_seen >= 4) {
                repetitions++;
                chunks_seen = 0;
            }

            if (pause_repetitions >= 4 && last_detected_key != -1 && (chunks_seen >= 3 || repetitions > 0)) {
                repetitions += (chunks_seen >= 3) ? 1 : 0;
                char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
                (*out_message)[message_length++] = letter;
first_letter_decoded = true;

                last_detected_key = -1;
                repetitions       = 0;
                chunks_seen       = 0;
                pause_repetitions = 0;
                key_cooldown      = debounce_window;
            }
        }

        if (key_cooldown > 0) {
            key_cooldown--;
        }
    }

    if (last_detected_key != -1 && (chunks_seen >= 3 || repetitions > 0)) {
    repetitions += (chunks_seen >= 3) ? 1 : 0;
    char letter = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
    (*out_message)[message_length++] = letter;
}

    return message_length;
}


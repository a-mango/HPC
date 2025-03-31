/**
 * @file dtmf_decode_goertzel.c
 * @brief DTMF decoding functions implementation using Goertzel algorithm.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "dtmf.h"
#include "dtmf_common.h"
#include "dtmf_error.h"
#include "dtmf_utils.h"


#define GOE_MAX_SAMPLES 3800

// Magic numbers
#define GOE_NOISE_THRESHOLD 44
#define PREPROCESS_THRESHOLD_FACTOR 1.1

static dtmf_float_t goertzel_detect(dtmf_float_t const *samples, dtmf_count_t num_samples, dtmf_float_t target_freq, dtmf_float_t sample_rate) {
    int          k      = (int)(0.5 + (((dtmf_float_t)num_samples * target_freq) / sample_rate));
    dtmf_float_t omega  = (M_2_PI * k) / (dtmf_float_t)num_samples;
    dtmf_float_t sine   = sin(omega);
    dtmf_float_t cosine = cos(omega);
    dtmf_float_t coeff  = 2.0 * cosine;
    dtmf_float_t q0 = 0, q1 = 0, q2 = 0;

    // Precompute window function values
    static dtmf_float_t window[GOE_MAX_SAMPLES];
    static int          window_initialized = 0;
    if (!window_initialized) {
        for (dtmf_count_t i = 0; i < GOE_MAX_SAMPLES; i++) {
            window[i] = 0.54 - 0.46 * cos((2 * M_PI * (dtmf_float_t)i) / (dtmf_float_t)(GOE_MAX_SAMPLES - 1));
        }
        window_initialized = 1;
    }

    for (dtmf_count_t i = 0; i < num_samples; i++) {
        dtmf_float_t windowed_sample = samples[i] * window[i];
        q0                           = coeff * q1 - q2 + windowed_sample;
        q2                           = q1;
        q1                           = q0;
    }

    dtmf_float_t real      = (q1 - q2 * cosine);
    dtmf_float_t imag      = (q2 * sine);
    dtmf_float_t magnitude = sqrt(real * real + imag * imag);

    return magnitude;
}

static void process_window(dtmf_float_t *dtmf_buffer, dtmf_count_t window_index, dtmf_count_t window_size, dtmf_float_t *max_magnitude, int *detected_key) {
    *max_magnitude = 0;
    *detected_key  = -1;

    for (int key = 1; key <= DTMF_NUM_TONES; key++) {
        dtmf_float_t magnitude =
            goertzel_detect(dtmf_buffer + window_index, window_size, DTMF_FREQUENCIES_MAP[key - 1].low, DTMF_SAMPLE_RATE_HZ) +
            goertzel_detect(dtmf_buffer + window_index, window_size, DTMF_FREQUENCIES_MAP[key - 1].high, DTMF_SAMPLE_RATE_HZ);

        if (magnitude > *max_magnitude) {
            *max_magnitude = magnitude;
            *detected_key  = key;
        }
    }

    if (*max_magnitude < GOE_NOISE_THRESHOLD) {
        *detected_key = -1;
    }
}

static void handle_detected_key(int detected_key, int *last_detected_key, dtmf_count_t *chunks_seen, dtmf_count_t *repetitions, dtmf_count_t *pause_repetitions, dtmf_count_t *message_length, char **out_message, dtmf_count_t debounce_window, dtmf_count_t *key_cooldown) {
    if (detected_key != -1) {
        if (*last_detected_key == -1) {
            *last_detected_key = detected_key;
            *chunks_seen       = 1;
        } else if (*last_detected_key == detected_key) {
            (*chunks_seen)++;
        } else {
            *chunks_seen       = 1;
            *repetitions       = 0;
            *last_detected_key = detected_key;
        }
        *pause_repetitions = 0;
    } else {
        (*pause_repetitions)++;

        if (*chunks_seen >= 4) {
            (*repetitions)++;
            *chunks_seen = 0;
        }

        if (*pause_repetitions >= 4 && *last_detected_key != -1 && (*chunks_seen >= 3 || *repetitions > 0)) {
            *repetitions += (*chunks_seen >= 3) ? 1 : 0;
            char letter                         = _dtmf_map_presses_to_letter((dtmf_count_t)*last_detected_key, *repetitions);
            (*out_message)[(*message_length)++] = letter;

            *last_detected_key = -1;
            *repetitions       = 0;
            *chunks_seen       = 0;
            *pause_repetitions = 0;
            *key_cooldown      = debounce_window;
        }
    }

    if (*key_cooldown > 0) {
        (*key_cooldown)--;
    }
}

bool dtmf_decode(dtmf_float_t *dtmf_buffer, dtmf_count_t const frame_count, char **out_message, dtmf_count_t *out_chars_read) {
    LIKWID_MARKER_START("dtmf-decode-goertzel");

    assert(dtmf_buffer != NULL);
    assert(out_message != NULL);
    assert(out_chars_read != NULL);
    assert(frame_count > 0);

    const dtmf_count_t window_size = DTMF_TONE_REPEAT_NUM_SAMPLES;
    const dtmf_count_t stride_size = DTMF_TONE_REPEAT_NUM_SAMPLES / 2;

    dtmf_count_t message_length     = 0;
    dtmf_count_t max_message_length = frame_count / stride_size + 1;
    *out_message                    = (char *)calloc(max_message_length, sizeof(char));
    if (!*out_message) {
        fprintf(stderr, "Error: Could not allocate memory for output message\n");
        return 0;
    }

    int                last_detected_key = -1;
    dtmf_count_t       chunks_seen       = 0;
    dtmf_count_t       repetitions       = 0;
    dtmf_count_t       pause_repetitions = 0;
    dtmf_count_t       key_cooldown      = 0;
    dtmf_count_t const debounce_window   = DTMF_TONE_NUM_SAMPLES / stride_size;

    _dtmf_preprocess_buffer(dtmf_buffer, frame_count, PREPROCESS_THRESHOLD_FACTOR);

    for (dtmf_count_t i = 0; i + window_size <= frame_count; i += stride_size) {
        dtmf_float_t max_magnitude;
        int          detected_key;

        process_window(dtmf_buffer, i, window_size, &max_magnitude, &detected_key);

        debug_printf("Window at %lu detected key %d (mag=%.2f)", i, detected_key, max_magnitude);

        handle_detected_key(detected_key, &last_detected_key, &chunks_seen, &repetitions, &pause_repetitions, &message_length, out_message, debounce_window, &key_cooldown);
    }

    if (last_detected_key != -1 && (chunks_seen >= 3 || repetitions > 0)) {
        repetitions += (chunks_seen >= 3) ? 1 : 0;
        char letter                      = _dtmf_map_presses_to_letter((dtmf_count_t)last_detected_key, repetitions);
        (*out_message)[message_length++] = letter;
    }

    *out_chars_read = message_length;

    LIKWID_MARKER_STOP("dtmf-decode-goertzel");

    DTMF_SUCCEED();
}

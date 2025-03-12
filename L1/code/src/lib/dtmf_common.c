/**
 * @file dtmf_common.c
 * @brief Common library functions.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include "dtmf_common.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "io_utils.h"


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
    {'2', 2,  4, {0, {0}}},
    {'A', 2,  1, {0, {0}}},
    {'B', 2,  2, {0, {0}}},
    {'C', 2,  3, {0, {0}}},
    {'3', 3,  4, {0, {0}}},
    {'D', 3,  1, {0, {0}}},
    {'E', 3,  2, {0, {0}}},
    {'F', 3,  3, {0, {0}}},
    {'4', 4,  4, {0, {0}}},
    {'G', 4,  1, {0, {0}}},
    {'H', 4,  2, {0, {0}}},
    {'I', 4,  3, {0, {0}}},
    {'5', 5,  4, {0, {0}}},
    {'J', 5,  1, {0, {0}}},
    {'K', 5,  2, {0, {0}}},
    {'L', 5,  3, {0, {0}}},
    {'6', 6,  4, {0, {0}}},
    {'M', 6,  1, {0, {0}}},
    {'N', 6,  2, {0, {0}}},
    {'O', 6,  3, {0, {0}}},
    {'7', 7,  6, {0, {0}}},
    {'P', 7,  1, {0, {0}}},
    {'Q', 7,  2, {0, {0}}},
    {'R', 7,  3, {0, {0}}},
    {'S', 7,  4, {0, {0}}},
    {'8', 8,  4, {0, {0}}},
    {'T', 8,  1, {0, {0}}},
    {'U', 8,  2, {0, {0}}},
    {'V', 8,  3, {0, {0}}},
    {'9', 9,  5, {0, {0}}},
    {'W', 9,  1, {0, {0}}},
    {'X', 9,  2, {0, {0}}},
    {'Y', 9,  3, {0, {0}}},
    {'Z', 9,  4, {0, {0}}},
    {'#', 10, 5, {0, {0}}},
    {'.', 10, 1, {0, {0}}},
    {'!', 10, 2, {0, {0}}},
    {'?', 10, 3, {0, {0}}},
    {',', 10, 4, {0, {0}}},
    {'0', 11, 2, {0, {0}}},
    {' ', 11, 1, {0, {0}}},
};

int dtmf_table_size = sizeof(dtmf_table_global) / sizeof(dtmf_table_global[0]);

DtmfTone dtmf_tones_map_global[DTMF_NUM_TONES];

static bool dtmf_initialized       = false;
static bool dtmf_table_initialized = false;
static bool dtmf_tones_initialized = false;

void _dtmf_gen_tone(DtmfFrequencyPair const frequencies, dtmf_float_t *buffer) {
    assert(buffer != NULL);

    for (size_t n = 0; n < DTMF_TONE_NUM_SAMPLES; n++) {
        dtmf_float_t t = (dtmf_float_t)n / (dtmf_float_t)DTMF_SAMPLE_RATE_HZ;

        buffer[n] = (DTMF_AMPLITUDE) * (sin(2 * M_PI * frequencies.low * t) +
                                        sin(2 * M_PI * frequencies.high * t));
    }
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
    debug_printf("Tones map initialized\n");
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
    debug_printf("Mappings table initialized\n");
}

void _dtmf_init() {
    if (dtmf_initialized) {
        return;
    }

    _dtmf_init_tones_map();
    _dtmf_init_table();

    dtmf_initialized = true;

    debug_printf("DTMF module initialized\n");
}

bool _dtmf_allocate_buffer(dtmf_float_t **buffer, size_t num_samples) {
    assert(buffer != NULL);

    *buffer = (dtmf_float_t *)malloc(num_samples * sizeof(dtmf_float_t));

    if (*buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate buffer of size %lu\n", num_samples);
        return false;
    }

    debug_printf("Allocated buffer of size %lu x %lu bytes\n", num_samples, sizeof(dtmf_float_t));
    return true;
}

char _dtmf_map_presses_to_letter(dtmf_count_t key, dtmf_count_t presses) {
    for (int i = 0; i < dtmf_table_size; i++) {
        if (dtmf_table_global[i].key == key && dtmf_table_global[i].presses == presses) {
            return dtmf_table_global[i].letter;
        }
    }

    debug_printf("No matching letter for key %lu with %lu presses.\n", key, presses);
    return '?';  // Return a placeholder if no match is found
}

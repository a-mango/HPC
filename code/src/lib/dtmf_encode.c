/**
 * @file dtmf_encode.c
 * @brief DTMF encoding functions implementation.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "dtmf_common.h"
#include "dtmf_utils.h"


static DtmfMapping  _dtmf_map_char(char const letter);
static bool         _dtmf_input_validate(char const *str);
static char        *_dtmf_input_normalize(char const *str);
static dtmf_count_t _dtmf_calc_duration(char const *message);
static bool         _dtmf_encode_message(char const *message, dtmf_float_t *dtmf_buffer);


static DtmfMapping _dtmf_map_char(char const letter) {
    for (int i = 0; i < dtmf_table_size; i++) {
        if (dtmf_table_global[i].letter == letter) {
            return dtmf_table_global[i];
        }
    }

    debug_printf("Letter %c not found in DTMF table.\n", letter);
    assert(false);  // FIXME: we probably want to bail at this point.

    return dtmf_table_global[dtmf_table_size - 1];  // DTMF_TONE_UNUSED
}

static bool _dtmf_input_validate(char const *str) {
    debug_printf("Validating input\n");

    for (char const *c = str; *c != '\0'; c++) {
        // TODO: extract to an allow list.
        if (!isalnum(*c) && !isdigit(*c) && *c != '!' && *c != '?' && *c != '.' && *c != ',' && *c != ' ' && *c != '#') {
            fprintf(stderr, "Error: Invalid character in message: %c\n", *c);
            return false;
        }
    }

    return true;
}

static char *_dtmf_input_normalize(char const *str) {
    char *normalized = calloc(strlen(str) + 1, sizeof(char));
    strcpy(normalized, str);

    for (char *c = normalized; *c != '\0'; c++) {
        *c = (char)toupper(*c);
    }

    debug_printf("Normalized message to: %s\n", normalized);

    return normalized;
}

static dtmf_count_t _dtmf_calc_duration(char const *message) {
    debug_printf("Calculating signal duration\n");
    debug_printf("Letter\tKey\tPresses\n");

    dtmf_count_t duration_total = 0;
    // Iterate over the chars of the message to compute it's duration.
    for (char const *c = message; *c != 0; c++) {
        DtmfMapping letter = _dtmf_map_char(*c);
        duration_total += letter.presses * DTMF_TONE_DURATION_MS;

        debug_printf("%c\t\t%d\t%d\n", letter.letter, letter.key, letter.presses);

        if (letter.presses > 1) {
            duration_total += (dtmf_count_t)(letter.presses - 1) * DTMF_TONE_REPEAT_MS;
        }

        if (*(c + 1) != '\0') {
            duration_total += DTMF_TONE_PAUSE_MS;
        }
    }

    return duration_total;
}

static bool _dtmf_encode_message(char const *message, dtmf_float_t *dtmf_buffer) {
    debug_printf("Processing message %s\n", message);

    size_t buffer_write_ptr = 0;  // This should track where we are in the buffer
    for (char const *c = message; *c != '\0'; c++) {
        DtmfMapping mapping = _dtmf_map_char(*c);

        // Write the tone by copying from the global table.
        memcpy(dtmf_buffer + buffer_write_ptr, mapping.tone.samples, mapping.tone.sample_count * sizeof(dtmf_float_t));
        buffer_write_ptr += mapping.tone.sample_count;

        // Add a pause between tones if there are more letters.
        if (*(c + 1) != '\0') {
            memset(dtmf_buffer + buffer_write_ptr, 0, DTMF_TONE_PAUSE_NUM_SAMPLES * sizeof(dtmf_float_t));
            buffer_write_ptr += DTMF_TONE_PAUSE_NUM_SAMPLES;
        }
    }

    return true;
}

// Encodes a message into a DTMF signal.
dtmf_count_t dtmf_encode(char const *message, dtmf_float_t **dtmf_buffer) {
    assert(message != NULL);
    assert(dtmf_buffer != NULL);

    debug_printf("Encoding message %s to DTMF...\n", message);
    _dtmf_init();

    if (!_dtmf_input_validate(message)) {
        fprintf(stderr, "Error: Invalid input message\n");
        return 0;  // TODO: find a better way to handle this, maybe using out params.
    }

    char const *normalized_input = _dtmf_input_normalize(message);

    dtmf_count_t duration_ms = _dtmf_calc_duration(normalized_input);
    dtmf_count_t num_samples = (duration_ms * DTMF_SAMPLE_RATE_HZ / DTMF_MS_PER_HZ);

    debug_printf("Signal duration @%d Hz: %lu ms\n", DTMF_SAMPLE_RATE_HZ, duration_ms);
    debug_printf("Number of samples: %lu\n", num_samples);

    if (!_dtmf_allocate_buffer(dtmf_buffer, num_samples)) {
        free((void *)normalized_input);
        exit(EXIT_FAILURE);
    }

    _dtmf_encode_message(normalized_input, *dtmf_buffer);

    free((void *)normalized_input);

    return num_samples;
}

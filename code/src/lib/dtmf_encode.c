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
#include "dtmf_error.h"
#include "dtmf_utils.h"

#define IS_DTMF_SYMBOL(symbol) (isalnum(symbol) || isdigit(symbol) || symbol == '!' || symbol == '?' || symbol == '.' || symbol == ',' || symbol == ' ' || symbol == '#' || symbol == '*')

static bool _dtmf_map_char(char const letter, DtmfMapping *out_mapped_char);
static bool _dtmf_input_validate(char const *str);
static bool _dtmf_input_normalize(char const *str, char **normalized_str);
static bool _dtmf_calc_duration(char const *message, dtmf_count_t *out_count);
static bool _dtmf_encode_message(char const *message, dtmf_float_t *dtmf_buffer);


static bool _dtmf_map_char(char const letter, DtmfMapping *out_mapped_char) {
    DTMF_DEBUG("Mapping letter %c to DTMF key\n", letter);
    assert(IS_DTMF_SYMBOL(letter));
    assert(out_mapped_char != NULL);

    for (int i = 0; i < dtmf_table_size; i++) {
        if (dtmf_table_global[i].letter == letter) {
            *out_mapped_char = dtmf_table_global[i];
            DTMF_TRACE("Mapped %c to %d x %d\n", letter, out_mapped_char->key, out_mapped_char->presses);
            DTMF_SUCCEED();
        }
    }

    DTMF_DEBUG("Letter %c not found in DTMF table.\n", letter);
    DTMF_ERROR("invalid character in message: %c", letter);
}

static bool _dtmf_input_validate(char const *str) {
    DTMF_DEBUG("Validating input\n");
    assert(str != NULL);

    for (char const *c = str; *c != '\0'; c++) {
        if (!IS_DTMF_SYMBOL(*c)) {
            DTMF_ERROR("invalid character in message: %c", *c);
        }
    }

    DTMF_SUCCEED();
}

static bool _dtmf_input_normalize(char const *str, char **out_normalized) {
    DTMF_DEBUG("Normalizing input\n");
    assert(str != NULL);
    assert(out_normalized != NULL);

    *out_normalized = (char *)calloc(strlen(str) + 1, sizeof(char));
    if (*out_normalized == NULL) {
        DTMF_ERROR("could not allocate memory for normalized message");
    }

    strcpy(*out_normalized, str);

    for (char *c = *out_normalized; *c != '\0'; c++) {
        *c = (char)toupper(*c);
    }

    DTMF_DEBUG("Normalized message to: %s\n", *out_normalized);

    DTMF_SUCCEED();
}

static bool _dtmf_calc_duration(char const *message, dtmf_count_t *out_count) {
    DTMF_DEBUG("Computing audio duration\n");
    assert(message != NULL);
    assert(out_count != NULL);

    DTMF_DEBUG("Letter\tKey\tPresses\n");

    dtmf_count_t duration_total = 0;
    // Iterate over the chars of the message to compute it's duration.
    for (char const *c = message; *c != 0; c++) {
        DtmfMapping letter;
        if (_dtmf_map_char(*c, &letter)) {
            DTMF_FAIL();
        }
        duration_total += letter.presses * DTMF_TONE_DURATION_MS;

        if (letter.presses > 1) {
            duration_total += (dtmf_count_t)(letter.presses - 1) * DTMF_TONE_REPEAT_MS;
        }

        if (*(c + 1) != '\0') {
            duration_total += DTMF_TONE_PAUSE_MS;
        }
    }

    if (duration_total == 0) {
        DTMF_ERROR("could not calculate duration of message");
    }

    *out_count = duration_total;
    DTMF_DEBUG("Computed duration: %lums\n", duration_total);
    DTMF_SUCCEED();
}

static bool _dtmf_encode_message(char const *message, dtmf_float_t *dtmf_buffer) {
    assert(message != NULL);
    assert(dtmf_buffer != NULL);
    DTMF_DEBUG("Processing message %s\n", message);


    size_t buffer_write_ptr = 0;
    for (char const *c = message; *c != '\0'; c++) {
        DtmfMapping mapping;

        if (!_dtmf_map_char(*c, &mapping)) {
            DTMF_FAIL();
        }

        // Write the tone by copying from the global table.
        memcpy(dtmf_buffer + buffer_write_ptr, mapping.tone.samples, mapping.tone.sample_count * sizeof(dtmf_float_t));
        DTMF_ASSERT(mapping.tone.sample_count != 0, "could not copy tone samples");
        buffer_write_ptr += mapping.tone.sample_count;

        // Add a pause between tones if there are more letters.
        if (*(c + 1) != '\0') {
            memset(dtmf_buffer + buffer_write_ptr, 0, DTMF_TONE_PAUSE_NUM_SAMPLES * sizeof(dtmf_float_t));
            DTMF_ASSERT(DTMF_TONE_PAUSE_NUM_SAMPLES != 0, "could not add pause between tones");
            buffer_write_ptr += DTMF_TONE_PAUSE_NUM_SAMPLES;
        }
    }

    DTMF_SUCCEED();
}

// Encodes a message into a DTMF signal.
dtmf_count_t dtmf_encode(char const *message, dtmf_float_t **dtmf_buffer) {
    assert(message != NULL);
    assert(dtmf_buffer != NULL);
    DTMF_DEBUG("Encoding message %s to DTMF...\n", message);

    _dtmf_init();

    if (_dtmf_input_validate(message)) {
        return 0;  // TODO: find a better way to handle this, maybe using out params.
    }

    char *normalized_input = NULL;
    if (_dtmf_input_normalize(message, &normalized_input)) {
        DTMF_FAIL();
    }

    dtmf_count_t duration_ms;
    if (_dtmf_calc_duration(normalized_input, &duration_ms)) {
        DTMF_FAIL();
    }

    DTMF_ASSERT(duration_ms != 0, "could not calculate duration of message");

    dtmf_count_t num_samples = (duration_ms * DTMF_SAMPLE_RATE_HZ / DTMF_MS_PER_HZ);

    DTMF_DEBUG("Signal duration @%d Hz: %lu ms\n", DTMF_SAMPLE_RATE_HZ, duration_ms);
    DTMF_DEBUG("Number of samples: %lu\n", num_samples);

    if (_dtmf_allocate_buffer(dtmf_buffer, num_samples)) {
        _dtmf_free_buffer(*dtmf_buffer);
        DTMF_FATAL("could not allocate buffer for DTMF signal");
    }

    _dtmf_encode_message(normalized_input, *dtmf_buffer);

    free(normalized_input);

    DTMF_DEBUG("Successfully encoded message of length %lums to DTMF signal\n", duration_ms);

    return num_samples;
}

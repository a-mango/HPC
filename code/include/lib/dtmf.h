/**
 * @file dtmf.h
 * @brief DTMF encoding and decoding public library interface.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-02-26
 */

#ifndef DTMF_H
#define DTMF_H

#include <math.h>
#include <stdint.h>

typedef double_t      dtmf_float_t;
typedef uint_fast64_t dtmf_count_t;

// Encodes a message into a DTMF signal.
dtmf_count_t dtmf_encode(char const *message, dtmf_float_t **dtmf_buffer);

// Decodes a DTMF signal into string.
bool dtmf_decode(dtmf_float_t *dtmf_buffer, dtmf_count_t const frame_count, char **out_message, dtmf_count_t *out_chars_read);


#endif  // DTMF_H

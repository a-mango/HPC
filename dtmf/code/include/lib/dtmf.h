/**
 * @file dtmf.h
 * @brief DTMF encoding and decoding public library interface.
 * @details
 * This file provides the public interface for encoding and decoding DTMF (Dual-Tone Multi-Frequency) signals.
 * It includes functions for encoding a message into a DTMF signal and decoding a DTMF signal back into a message.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-02-26
 */

#ifndef DTMF_H
#define DTMF_H

#include <math.h>
#include <stdint.h>

typedef double_t      dtmf_float_t;
typedef uint_fast64_t dtmf_count_t;

/**
 * @brief Encodes amessage into a DTMF signal.
 * @details
 * This function encodes a message into a DTMF signal. It pre-processes the input message, normalizes it,
 * then encodes it into a DTMF signal. The encoded signal is stored as doubles in the output buffer.
 * @warning The output buffer must NOT be allocated on input, and must be freed by the caller.
 *
 * @param message The message to encode.
 * @param dtmf_buffer The output buffer to store the encoded DTMF signal.
 * @param out_samples_count The number of samples in the encoded DTMF signal.
 * @return true if encoding was successful, false otherwise.
 */
bool dtmf_encode(char const *message, dtmf_float_t **out_dtmf_buffer, dtmf_count_t *out_samples_count);

/**
 * @brief Decodes a DTMF signal into a message.
 * @details
 * This function decodes a DTMF signal into a message. It pre-processes then processes the input buffer
 * of samples and extracts the DTMF tones if present. The decoded message is stored in the output buffer.
 * This process is done using the Goertzel or Fast Fourier Transform algorithms, depending on the linkage.
 * @warning The output buffer must NOT be allocated on input, and must be freed by the caller.
 *
 * @param dtmf_buffer The input buffer of samples to decode.
 * @param frame_count The number of frames in the buffer
 * @param out_message The output buffer to store the decoded message. Must NOT be allocated.
 * @param out_chars_read The number of characters read from the DTMF signal.
 * @return true if decoding was successful, false otherwise.
 */
bool dtmf_decode(dtmf_float_t *dtmf_buffer, dtmf_count_t const frame_count, char **out_message, dtmf_count_t *out_chars_read);


#endif  // DTMF_H

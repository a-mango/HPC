/**
 * @file dtmf_utils.c
 * @brief Utilities for the DTMF enc/decoder.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-02-26
 */

#ifndef UTILS_H
#define UTILS_H

#include <sndfile.h>

#ifdef LIKWID_PERFMON
#include <likwid-marker.h>
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

#ifdef DEBUG
#define debug_printf(fmt, ...) DTMF_DEBUG(fmt, __VA_ARGS__)
#else
#define debug_printf(...) ((void)0)
#endif


/**
 * @brief Reads a text file into a buffer.
 *
 * @param filename The name of the file to read.
 * @param out_buffer The NULL-initialized buffer to write the file content to.
 * @return A boolean indicating the success of the operation.
 */
bool utils_read_text_file(char const *filename, char **out_buffer);

/**
 * @brief Writes a buffer to a WAV file.
 *
 * @param filename The name of the file to write to.
 * @param in_buffer A NULL-initialized buffer to write to the file.
 * @param frames The number of frames to write.
 * @return A boolean indicating the success of the operation.
 */
bool utils_read_wav_file(char const *filename, double **out_buffer, SF_INFO *out_sfinfo);


/**
 * @brief Writes a buffer of floating-point frames to a WAV file.
 *
 * @param filename The name of the file to write to.
 * @param values_hz The buffer of frames in herz as floating-point units.
 * @param frames The number of frames to write.
 * @return A boolean indicating the success of the operation.
 */
bool utils_write_wav_file(char const *filename, double *values_hz, sf_count_t frames);

/**
 * @brief Prints the usage of the program.
 *
 * @param prog The name of the program.
 */
void utils_print_usage(char const *prog);

#endif  // UTILS_H

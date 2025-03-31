/**
 * @file main.c
 * @brief Main program for DTMF encoding and decoding.
 * @date 2025-02-26
 */

#include <dtmf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "dtmf_utils.h"
#include "lib/dtmf_error.h"

/*
 * Program entry point.
 */
int main(int argc, char *argv[]) {
    LIKWID_MARKER_INIT;

    struct arguments arguments;
    parse_arguments(argc, argv, &arguments);

    if (strcmp(arguments.command, "encode") == 0) {
        char *buffer = NULL;
        if (!utils_read_text_file(arguments.input, &buffer)) {
            fprintf(stderr, "Error: Could not read file %s\n", arguments.input);
            return EXIT_FAILURE;
        }

        dtmf_float_t *dtmf_buffer = NULL;
        dtmf_count_t  samples_cnt = 0;
        if (dtmf_encode(buffer, &dtmf_buffer, &samples_cnt)) {
            free(buffer);
            DTMF_EXIT_FAILURE();
        }

        if (!utils_write_wav_file(arguments.output, dtmf_buffer, (sf_count_t)samples_cnt)) {
            fprintf(stderr, "Error: Could not write to file %s\n", arguments.output);
            free(buffer);
            free(dtmf_buffer);
            DTMF_EXIT_FAILURE();
        }

        free(buffer);
        free(dtmf_buffer);
    } else if (strcmp(arguments.command, "decode") == 0) {
        SF_INFO       sf_info     = {0};
        dtmf_float_t *dtmf_buffer = NULL;

        if (!utils_read_wav_file(arguments.input, &dtmf_buffer, &sf_info)) {
            fprintf(stderr, "Error: Could not read file %s\n", arguments.input);
            DTMF_EXIT_FAILURE();
        }

        char        *message    = NULL;
        dtmf_count_t count_read = 0;

        LIKWID_MARKER_START("dtmf-decode");
        if (dtmf_decode(dtmf_buffer, (dtmf_count_t)sf_info.frames, &message, &count_read)) {
            DTMF_EXIT_FAILURE();
        }
        LIKWID_MARKER_STOP("dtmf-decode");


        if (message != NULL) {
            printf("%s\n", message);
            free(message);  // FIXME: use lib allocators
        } else {
            fprintf(stderr, "Error: Decoding failed\n");
        }

        if (dtmf_buffer != NULL) {
            free(dtmf_buffer);  // FIXME: use lib allocators
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", arguments.command);
        DTMF_EXIT_FAILURE();
    }

    LIKWID_MARKER_CLOSE;

    return EXIT_SUCCESS;
}

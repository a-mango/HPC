/**
 * @file main.c
 * @brief Main program for DTMF encoding and decoding.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-02-26
 */

#include <dtmf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_utils.h"

/*
 * Program entry point.
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        utils_print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "encode") == 0) {
        if (argc != 4) {
            utils_print_usage(argv[0]);
            return 1;
        }

        char *buffer = NULL;
        if (!utils_read_text_file(argv[2], &buffer)) {
            fprintf(stderr, "Error: Could not read file %s\n", argv[2]);
            return EXIT_FAILURE;
        }

        dtmf_float_t *dtmf_buffer      = NULL;
        dtmf_count_t  dtmf_frame_count = dtmf_encode(buffer, &dtmf_buffer);

        utils_write_wav_file(argv[3], dtmf_buffer, (sf_count_t)dtmf_frame_count);

        free(buffer);
        free(dtmf_buffer);
    } else if (strcmp(argv[1], "decode") == 0) {
        SF_INFO       sf_info     = {0};
        dtmf_float_t *dtmf_buffer = NULL;

        if (!utils_read_wav_file(argv[2], &dtmf_buffer, &sf_info) != 0) {
            fprintf(stderr, "Error: Could not read file %s\n", argv[2]);
            return EXIT_FAILURE;
        }

        char *message = NULL;
        dtmf_decode(dtmf_buffer, &message, (dtmf_count_t)sf_info.frames);

        printf("%s\n", message);

        free(dtmf_buffer);
        free(message);
    } else {
        utils_print_usage(argv[0]);
        return 1;
    }

    return 0;
}

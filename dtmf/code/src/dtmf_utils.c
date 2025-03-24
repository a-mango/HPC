/**
 * @file io_utils.h
 * @brief IO functions implementation.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#include <assert.h>
#include <sndfile.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dtmf_utils.h"

bool utils_read_text_file(char const *filename, char **out_buffer) {
    assert(filename != NULL);
    assert(out_buffer != NULL);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    *out_buffer = calloc(file_size + 1, sizeof(char));
    if (!*out_buffer) {
        fprintf(stderr, "Error: could not allocate memory for file content\n");
        fclose(file);
        return false;
    }

    if (fread(*out_buffer, 1, file_size, file) != file_size) {
        fprintf(stderr, "Error: could not read file content\n");
        fclose(file);
        return false;
    }

    if (fclose(file) < 0) {
        fprintf(stderr, "Error: could not close file\n");
        return false;
    }

    return true;
}

bool utils_read_wav_file(char const *filename, double **out_buffer, SF_INFO *out_sfinfo) {
    assert(filename != NULL);
    assert(out_buffer != NULL);
    assert(out_sfinfo != NULL);

    debug_printf("Reading file %s...\n", filename);

    SNDFILE *infile = sf_open(filename, SFM_READ, out_sfinfo);
    if (!infile) {
        fprintf(stderr, "Error: could not open file '%s': %s\n", filename, sf_strerror(NULL));
        return 1;
    }

    SF_FORMAT_INFO format_info;
    format_info.format = out_sfinfo->format;
    sf_command(infile, SFC_GET_FORMAT_INFO, &format_info, sizeof(format_info));

    debug_printf("File information:\n");
    debug_printf("  - Format: %08x  %s %s\n", format_info.format, format_info.name, format_info.extension);
    debug_printf("  - Channel count: %d\n", out_sfinfo->channels);
    debug_printf("  - Sample rate: %d Hz\n", out_sfinfo->samplerate);
    debug_printf("  - Sample count: %lld\n", (long long)out_sfinfo->frames);
    debug_printf("  - Duration: %.2f secondes\n", (double)out_sfinfo->frames / out_sfinfo->samplerate);

    if ((out_sfinfo->format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        fprintf(stderr, "Error: file '%s' is not a WAV file.\n", filename);
        sf_close(infile);
        return false;
    }

    *out_buffer = (double *)malloc((size_t)out_sfinfo->frames * (size_t)out_sfinfo->channels * sizeof(double));
    if (!out_buffer) {
        fprintf(stderr, "Error: could not allocate memory for file content\n");
        sf_close(infile);
        return false;
    }

    sf_count_t frames_read = sf_readf_double(infile, *out_buffer, out_sfinfo->frames);
    if (frames_read != out_sfinfo->frames) {
        fprintf(stderr, "Warning: %zu frames out of %zu were read.\n", frames_read, out_sfinfo->frames);
    }

    sf_close(infile);

    debug_printf("Done reading file %s\n", filename);

    return true;
}

bool utils_write_wav_file(char const *filename, double *values_hz, sf_count_t frames) {
    assert(filename != NULL);
    assert(values_hz != NULL);

    debug_printf("Writing %lld frames to %s...\n", (long long)frames, filename);

    SF_INFO sfinfo    = {0};
    sfinfo.samplerate = 44100;
    sfinfo.channels   = 1;
    sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_DOUBLE;
    sfinfo.frames     = frames;

    SNDFILE *outfile = sf_open(filename, SFM_WRITE, &sfinfo);
    if (!outfile) {
        fprintf(stderr, "Error: couldn't create file '%s': %s\n", filename, sf_strerror(NULL));
        return false;
    }

    sf_count_t count = sf_writef_double(outfile, values_hz, frames);
    if (count != frames) {
        fprintf(stderr, "Warning: %lld frames out of %lld were written.\n", (long long)count, (long long)frames);
        sf_close(outfile);
        return false;
    }

    sf_write_sync(outfile);
    sf_close(outfile);

    debug_printf("Done writing file %s\n", filename);

    return true;
}

void utils_print_usage(char const *prog) {
    printf("Usage :\n  %s encode input.txt output.wav\n  %s decode input.wav\n", prog, prog);
}

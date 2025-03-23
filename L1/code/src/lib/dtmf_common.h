/**
 * @file dtmf_common.h
 * @brief Common library functions.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-11
 */

#ifndef DTMF_COMMON_H
#define DTMF_COMMON_H

#include <stddef.h>

#include "dtmf.h"

#define M_PI 3.1415926535897
#define M_PI_2 1.5707963267949
#define M_2PI 6.2831853071796

#define DTMF_SAMPLE_RATE_HZ 44100
#define DTMF_AMPLITUDE 1
#define DTMF_MS_PER_HZ 1000

#define DTMF_TONE_DURATION_MS 200
#define DTMF_TONE_PAUSE_MS 200
#define DTMF_TONE_REPEAT_MS 50

#define DTMF_MAX_SAMPLES ((DTMF_TONE_DURATION_MS) * (DTMF_SAMPLE_RATE_HZ))

#define DTMF_TONE_DURATION_HZ (DTMF_TONE_DURATION_MS * DTMF_SAMPLE_RATE_HZ / 1000)
#define DTMF_TONE_PAUSE_HZ (DTMF_TONE_PAUSE_MS * DTMF_SAMPLE_RATE_HZ / 1000)
#define DTMF_TONE_REPEAT_HZ (DTMF_TONE_REPEAT_MS * DTMF_SAMPLE_RATE_HZ / 1000)

#define DTMF_TONE_NUM_SAMPLES (DTMF_SAMPLE_RATE_HZ * DTMF_TONE_DURATION_MS / DTMF_MS_PER_HZ)
#define DTMF_TONE_PAUSE_NUM_SAMPLES (DTMF_SAMPLE_RATE_HZ * DTMF_TONE_PAUSE_MS / DTMF_MS_PER_HZ)
#define DTMF_TONE_REPEAT_NUM_SAMPLES (DTMF_SAMPLE_RATE_HZ * DTMF_TONE_REPEAT_MS / DTMF_MS_PER_HZ)
#define DTMF_TONE_MAX_SYMBOLS_PER_KEY 5
#define DTMF_TONE_MAX_SILENCE_PER_KEY 5
#define DTMF_TONE_NUM_SAMPLES_MAX (DTMF_TONE_MAX_SYMBOLS_PER_KEY * DTMF_TONE_NUM_SAMPLES + DTMF_TONE_MAX_SILENCE_PER_KEY * DTMF_TONE_REPEAT_HZ)

#define DTMF_FREQ_LOW_1 697
#define DTMF_FREQ_LOW_2 770
#define DTMF_FREQ_LOW_3 852
#define DTMF_FREQ_LOW_4 941
#define DTMF_FREQ_HIGH_1 1209
#define DTMF_FREQ_HIGH_2 1336
#define DTMF_FREQ_HIGH_3 1477

#define DTMF_LOW_FREQS_COUNT 4
#define DTMF_HIGH_FREQS_COUNT 3

#define DTMF_LOW_FREQS {DTMF_FREQ_LOW_1, DTMF_FREQ_LOW_2, DTMF_FREQ_LOW_3, DTMF_FREQ_LOW_4}
#define DTMF_HIGH_FREQS {DTMF_FREQ_HIGH_1, DTMF_FREQ_HIGH_2, DTMF_FREQ_HIGH_3}

#define DTMF_TONE_1 DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_1
#define DTMF_TONE_2 DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_2
#define DTMF_TONE_3 DTMF_FREQ_LOW_1, DTMF_FREQ_HIGH_3
#define DTMF_TONE_4 DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_1
#define DTMF_TONE_5 DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_2
#define DTMF_TONE_6 DTMF_FREQ_LOW_2, DTMF_FREQ_HIGH_3
#define DTMF_TONE_7 DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_1
#define DTMF_TONE_8 DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_2
#define DTMF_TONE_9 DTMF_FREQ_LOW_3, DTMF_FREQ_HIGH_3
#define DTMF_TONE_POUND DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_1
#define DTMF_TONE_0 DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_2
#define DTMF_TONE_UNUSED DTMF_FREQ_LOW_4, DTMF_FREQ_HIGH_3

#define DTMF_NUM_TONES 12

#define DTMF_UNKNOWN_SYMBOL '~'

constexpr dtmf_float_t dtmf_freqs_low_g[]  = DTMF_LOW_FREQS;
constexpr dtmf_float_t dtmf_freqs_high_g[] = DTMF_HIGH_FREQS;

typedef struct {
    dtmf_count_t sample_count;
    dtmf_float_t samples[DTMF_TONE_NUM_SAMPLES_MAX];
} DtmfTone;

typedef struct {
    dtmf_float_t low;
    dtmf_float_t high;
} DtmfFrequencyPair;

// TODO: optimize this by creating a vector only for tones.
typedef struct {
    char     letter;
    uint8_t  key;
    uint8_t  presses;
    DtmfTone tone;
} DtmfMapping;

// extern constexpr DtmfFrequencyPair DTMF_FREQUENCIES_MAP[DTMF_NUM_TONES];
extern DtmfMapping       dtmf_table_global[];
extern int               dtmf_table_size;
extern DtmfTone          dtmf_tones_map_global[DTMF_NUM_TONES];
extern DtmfFrequencyPair DTMF_FREQUENCIES_MAP[DTMF_NUM_TONES];

// Generates a DTMF tone from frequencies.
void _dtmf_gen_tone(DtmfFrequencyPair const frequencies, dtmf_float_t *buffer);

// Initialize the global DTMF tones map.
void _dtmf_init_tones_map();

// Initialize the tones contained in the global DTMF table.
void _dtmf_init_table();

void _dtmf_init();

// Allocate a new buffer of size num_samples for the DTMF signal.
bool _dtmf_allocate_buffer(dtmf_float_t **buffer, size_t num_samples);

// Translates a key and number of presses to a DTMF letter.
char _dtmf_map_presses_to_letter(dtmf_count_t key, dtmf_count_t presses);

// Compute the root mean square of the buffer.
dtmf_float_t _dtmf_compute_rms(const dtmf_float_t *buffer, dtmf_count_t num_samples);

// Apply noise reduction to the buffer.
void _dtmf_noise_reduction(dtmf_float_t *buffer, size_t num_samples, dtmf_float_t threshold_factor);

void _dtmf_normalize_signal(dtmf_float_t *buffer, dtmf_count_t count);

void _dtmf_apply_bandpass(dtmf_float_t *buffer, dtmf_count_t count);

dtmf_float_t _dtmf_calculate_noise_threshold(dtmf_float_t const *buffer, dtmf_count_t count);

void _dtmf_pre_emphasis(dtmf_float_t *buffer, dtmf_count_t count);

#endif  // DTMF_COMMON_H

/**
 * @file normalize.c
 * @brief Normalization function taken from the DTMF program.
 * @date 2025-04-08
 */

#include <assert.h>
#include <math.h>

#pragma GCC push_options
#pragma GCC optimize("-O0")
static void original(float *buffer, unsigned const count) {
    float max = 0.0;
    // Find maximum absolute value
    for (int i = 0; i < count; i++) {
        if (fabs(buffer[i]) > max)
            max = fabs(buffer[i]);
    }
    // Normalize if max is not zero
    if (max > 0.0001) {
        for (int i = 0; i < count; i++) {
            buffer[i] /= max;
        }
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize("-O0")
static void optimized(float *buffer, unsigned const count) {
    float max = 0.0;
    // Find maximum absolute value
    for (int i = 0; i < count; i++) {
        float abs_val = fabs(buffer[i]);
        if (abs_val > max)
            max = abs_val;
    }
    // Normalize if max is not zero
    if (max > 0.0001) {
        for (int i = 0; i < count; i++) {
            buffer[i] /= max;
        }
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize("-Ofast")
static void compiler(float *buffer, unsigned const count) {
    float max = 0.0;
    // Find maximum absolute value
    for (int i = 0; i < count; i++) {
        if (fabs(buffer[i]) > max)
            max = fabs(buffer[i]);
    }
    // Normalize if max is not zero
    if (max > 0.0001) {
        for (int i = 0; i < count; i++) {
            buffer[i] /= max;
        }
    }
}
#pragma GCC pop_options

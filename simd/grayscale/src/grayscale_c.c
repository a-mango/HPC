#include <stdint.h>
#include <stdlib.h>

#include "grayscale.h"

#define R_OFFSET 0
#define G_OFFSET 1
#define B_OFFSET 2

void grayscale(struct img_t *image) {
    const int ncomps  = image->components;
    const int npixels = image->width * image->height;
    uint8_t  *src     = image->data;

    // Allocate output buffer
    uint8_t *dst = malloc(npixels);
    if (!dst) {
        return;
    }

    for (int i = 0; i < npixels; ++i) {
        uint8_t r = src[ncomps * i + R_OFFSET];
        uint8_t g = src[ncomps * i + G_OFFSET];
        uint8_t b = src[ncomps * i + B_OFFSET];
        dst[i]    = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);  // Grayscale conversion
    }

    // Replace image data with grayscale buffer
    free(image->data);
    image->data       = dst;
    image->components = 1;
}

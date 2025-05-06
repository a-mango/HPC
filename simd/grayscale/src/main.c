#include <stdio.h>
#include <stdlib.h>

#include "grayscale.h"

int main(int argc, char** argv) {
    struct img_t* img;
    if (argc < 3) {
        fprintf(stderr, "Usage : %s <img_src.png> <img_dest.png>\n", argv[0]);
        return EXIT_FAILURE;
    }

    img = load_image(argv[1]);

    printf("Image loaded!\n");

    grayscale(img);

    save_image(argv[2], img);

    free_image(img);

    printf("Programm ended successfully\n\n");

    return 0;
}

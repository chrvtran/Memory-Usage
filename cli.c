#include "kernel.h"
#include <string.h>

int generate_pagefault() {

}

int main(int argc, char** argv){
    // TODO: parse the arguments in argv. 
    if(argc != 6) {
        printf("Incorrect number of arguments. Expected: ./cli <MODE=kernel|mmap|convert|uconvert|fault> <input_image> <width> <height> <output_image_path>\n");
        return -1;
    }

    char* mode = argv[1];
    char* in_path = argv[2]; // image input filepath
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    char* out_path = argv[5]; // image output filepath

    // TODO: allocate the space needed for one image and load the image
    struct image* loaded_image = calloc(1, sizeof(struct image));
    if (loaded_image == NULL) {
        printf("failed to allocate loaded_image");
        return 1; // failed allocation
    }
    loaded_image->height = height;
    loaded_image->width = width;
    if (loaded_image->pixels == NULL) {
        free(loaded_image);
        printf("failed to allocate loaded_image->pixels");
        return 1; // failed allocation
    }

    // TODO: call correct function based on mode
    if (strcmp(mode, "kernel") == 0) {
        loadimage(in_path, loaded_image);
    }

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}}; // kernel size is 3

    // TODO: call apply kernel with 1/9 (as a float) as the normalization value
    struct image* output = apply_kernel(loaded_image, &kernel[0][0], 3, (float) 1 / 9);

    free(loaded_image->pixels);
    free(loaded_image);
    free(output->pixels);
    free(output);
}

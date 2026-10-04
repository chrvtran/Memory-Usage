#include "kernel.h"
#include <string.h>
#include <sys/mman.h>

int generate_pagefault() {
    // making a valid image
    struct image* image = calloc(1, sizeof(struct image));
    if (image == NULL) {
        return -1;
    }
    image->height = 16;
    image->width = 16;
    image->pixels = calloc(1, sizeof(struct pixel) * 16 * 16);
    if (image->pixels == NULL) {
        free(image);
        return -1;
    }

    // the key, not on physical mem
    saveimage_mmap("yolo.bmp", image);

    free(image->pixels);
    free(image);

    return 0;
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

    if (strcmp(mode, "fault") == 0) {
        return generate_pagefault();
    }

    // checking base cases
    if (width <= 0 || height <= 0) return -1;
    if (strcmp(mode, "kernel") != 0 && strcmp(mode, "mmap") != 0 &&
        strcmp(mode, "convert") != 0 && strcmp(mode, "uconvert") != 0) {
        printf("wrong mode");
        return -1;
    }

    // TODO: allocate the space needed for one image and load the image
    struct image* loaded_image = calloc(1, sizeof(struct image));
    if (loaded_image == NULL) {
        printf("failed to allocate loaded_image");
        return 1;
    }
    loaded_image->height = height;
    loaded_image->width = width;

    // TODO: call correct function based on mode
    // image loading
    int load_result;
    if (strcmp(mode, "mmap") == 0 || strcmp(mode, "uconvert") == 0) {
        load_result = loadimage_mmap(in_path, loaded_image);
    } else {
        // kernel || convert
        load_result = loadimage(in_path, loaded_image);
    }
    if (load_result != 0) {
        printf("failed to load");
        free(loaded_image);
        return 1;
    }

    // saving for convert
    if (strcmp(mode, "convert") == 0) {
        if (saveimage_mmap(out_path, loaded_image) != 0) {
            printf("save mmap for convert failed");
            free(loaded_image->pixels);
            free(loaded_image);
            return 1;
        }

    // saving for uconvert
    } else if (strcmp(mode, "uconvert") == 0) {
        if (saveimage(out_path, loaded_image) != 0) {
            printf("save bmp for uconvert failed");
            munmap((char*)loaded_image->pixels - sizeof(struct image),
                    sizeof(struct image) + sizeof(struct pixel) * width * height);
            free(loaded_image);
            return 1;
        }
    }

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}}; // kernel size is 3

    // TODO: call apply kernel with 1/9 (as a float) as the normalization value
    struct image* output = NULL;
    if (strcmp(mode, "kernel") == 0 || strcmp(mode, "mmap") == 0){
        output = apply_kernel(loaded_image, &kernel[0][0], 3, (float) 1 / 9);

        // no output for some reason
        if (output == NULL) {
            printf("apply kernel has output NULL"); // start freeing stuff
            
            if (strcmp(mode, "mmap") == 0) {
                munmap((char*)loaded_image->pixels - sizeof(struct image),
                       sizeof(struct image) + sizeof(struct pixel) * width * height);
            } else {
                // free for kernel
                free(loaded_image->pixels);
            }
            free(loaded_image);
            return 1;

        // non-successful output for apply_kernel
        } else if (saveimage(out_path, output) != 0) {
            printf("output for apply_kernel failed");
            if (strcmp(mode, "mmap") == 0) {
                munmap((char*)loaded_image->pixels - sizeof(struct image),
                       sizeof(struct image) + sizeof(struct pixel) * width * height);
            } else {
                free(loaded_image->pixels);
            }
            free(loaded_image);
            free(output->pixels);
            free(output);
            return 1;
        }
    }

    // Freeing
    if (strcmp(mode, "uconvert") == 0 || strcmp(mode, "mmap") == 0) {
        munmap((char*)loaded_image->pixels - sizeof(struct image)
                , sizeof(struct image) + sizeof(struct pixel) * width * height);
    } else {
        free(loaded_image->pixels);
    }
    free(loaded_image);
    if (output != NULL) {
        free(output->pixels);
        free(output);
    }
}

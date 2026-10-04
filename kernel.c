#include "loader.h"
#include <stdlib.h>
#include <string.h>

/** Returns p1 with each channel multiplied by scalar. */
struct pixel mul(struct pixel p1, float scalar) {
    return (struct pixel){r: p1.r * scalar, g: p1.g * scalar, b: p1.b * scalar};
}
/** Returns the channel-wise sum of p1 and p2. */
struct pixel add(struct pixel p1, struct pixel p2) {
    return (struct pixel){r: p1.r + p2.r, g: p1.g + p2.g, b: p1.b + p2.b};
}

/**
 * Applies a square kernel to an image (cross-correlation).
 *
 * Produces a new image where each output pixel is the weighted sum of
 * the ksize x ksize neighborhood centered on the corresponding input
 * pixel, multiplied by normalize. The kernel is applied as-is (not
 * flipped), so this is technically cross-correlation; the result is
 * identical to convolution for symmetric kernels.
 *
 * The input img is padded so that kernel operations that fall outside of the 
 * original image are multiplied by a black pixel (zero padding).
 *
 * img        Source image. Not modified.
 * kernel     Kernel weights in row-major order, containing ksize * ksize elements.
 * ksize      Width and height of the kernel. Should be odd
 * normalize  Scale factor applied to each weighted sum
 *                       (e.g., 1.0f / 9 for a 3x3 box blur).
 *
 * Returns a pointer to a newly allocated image with the same dimensions as img.
 *
 */
struct image* apply_kernel(struct image* img, int* kernel, int ksize, float normalize) {
    int height = img->height;
    int width = img->width;

    // calloc output
    struct image* output = calloc(1, sizeof(struct image));
    if (output == NULL) {
        printf("failed to allocate output in apply_kernel");
        return NULL; // failed allocation
    }
    output->height = height;
    output->width = width;
    output->pixels = calloc(1, sizeof(struct pixel) * width * height);
    if (output->pixels == NULL) {
        printf("failed to allocate output->pixels in apply_kernel");
        free(output);
        return NULL; // failed allocation
    }

    int kr = ksize / 2; // reach around origin

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            struct pixel p = {0, 0, 0};

            for (int krow = 0; krow < ksize; krow++) { // on the kernel
                for (int kcol = 0; kcol < ksize; kcol++) {
                    int curw = w + kcol - kr;   // on image on this kernel
                    int curh = h + krow - kr;

                    struct pixel addp = {0, 0, 0}; // black pixel for off image pixel
                    if (curw >= 0 && curw < width && curh >= 0 && curh < height)
                        addp = img->pixels[curh * width + curw];

                    p = add(p, mul(addp, (float)kernel[krow * ksize + kcol]));
                }
            }

            p = mul(p, normalize);
            int r = (p.r < 0) ? 0 : (p.r > 255) ? 255 : p.r;
            int g = (p.g < 0) ? 0 : (p.g > 255) ? 255 : p.g;
            int b = (p.b < 0) ? 0 : (p.b > 255) ? 255 : p.b;
            output->pixels[h * width + w] = (struct pixel){r, g, b};
        }
    }

    return output;
}


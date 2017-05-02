//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>

struct Image {
    size_t width;
    size_t height;
    double **image;
};

/**
 * Typedef for easier usage
 */
typedef struct Image Image;

/**
 * Initialize an image
 *
 * @param width width of the image
 * @param height height of the image
 * @param default_value set to every value in the image
 * @return image pointer with the set value and the respective sizes
 */
static Image *init_image(size_t width, size_t height, double default_value);

/**
 * This Image is equal to the following Kernel
 *    2 -2  2 -2  2
 *   -2  2 -2  2 -2
 *    2 -2 -1 -2  2
 *   -2  2 -2  2 -2
 *    2 -2  2 -2  2
 *
 * @return Return default Kernel Image for comparison to the haskell version
 */
static Image* get_default_kernel(void);

/**
 * Free the allocated resources of an image pointer including the image struct itself
 * @param image Image which shall be freed
 */
static void free_image(Image *image);

/**
 * Helper function to print the image in an easy way to read
 * @param img Image that shall be printed, line based
 */
static void print_image(Image *img);

/**
 * Copy the dimensions of the given image.
 *
 * @param img Image that provides the dimensions for the new image
 * @return Image with the same dimensions as the old image
 */
static Image *copy_shape(Image *img);


int main(int argc, char **argv) {
    Image *image = init_image(10, 10, 1.0);
    Image* kernel = get_default_kernel();
    print_image(image);
    print_image(kernel);
    free_image(image);
    free_image(kernel);
    return 0;
}


static void free_image(Image *image) {
    if (image != NULL) {
        for (int height = 0; height < image->height; ++height) {
            if (image->image[height] != NULL) {
                free(image->image[height]);
            }
        }
        if (image->image != NULL) {
            free(image->image);
        }
        free(image);
    }
}

static Image *init_image(size_t width, size_t height, double default_val) {
    Image *img = (Image *) malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->image = (double **) malloc(sizeof(double *) * 10);
    for (int height = 0; height < img->height; ++height) {
        img->image[height] = (double *) malloc(sizeof(double) * 10);
        for (int width = 0; width < img->width; ++width) {
            img->image[height][width] = default_val;
        }
    }
    return img;
}

static void print_image(Image *img) {
    for (int height = 0; height < img->height; ++height) {
        for (int width = 0; width < img->width; ++width) {
            char *format = NULL;
            if (img->image[height][width] < 0) {
                format = "%.02f\t";
            } else {
                format = " %.02f\t";
            }
            printf(format, img->image[height][width]);
        }
        printf("\n");
    }
}

static Image *copy_shape(Image *img) {
    return init_image(img->width, img->height, 0.0);
}

static Image *get_default_kernel(void) {
    Image *img = init_image(5, 5, 0);
    for (int height = 0; height < img->height; ++height) {
        for (int width = 0; width < img->width; ++width) {
            if((height + width) % 2 == 0) {
                img->image[height][width] = 2;
            } else {
                img->image[height][width] = -2;
            }

            if (height == 2 && width == 2) {
                img->image[height][width] = -1;
            }
        }
    }
    return img;
}

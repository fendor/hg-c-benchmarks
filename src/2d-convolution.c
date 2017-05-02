//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>
#include <getopt.h>
#include "util.h"

struct Image {
    size_t width;
    size_t height;
    double **image;
};

struct Args {
    size_t numberOfIterations;
    size_t numberOfProcesses;
    size_t width;
    size_t height;
} args;

static char *pgmname;

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
static Image *get_default_kernel(void);

/**
 * Free the allocated resources of an image pointer including the image struct itself
 * @param image Image which shall be freed
 */
static void free_image(Image *image);

/**
 * Copy the dimensions of the given image.
 *
 * @param img Image that provides the dimensions for the new image
 * @return Image with the same dimensions as the old image
 */
static Image *copy_shape(Image *img);

/**
 * Apply a given kernel on every pixel of an image.
 * Computed values are written to the buffer.
 * Values that are outside of the image are set to the edge values
 *
 * @param img Image to which the kernel is applied
 * @param kernel Kernel to apply to each pixel of the image
 * @param buffer Output buffer
 */
static void apply_kernel_to_image(Image *img, Image *kernel, Image *buffer);

/**
 * Apply a given kernel on a pixel of an image.
 * Values that are outside of the image are set to the edge values
 *
 * @param img mage to which the kernel is applied
 * @param kernel Kernel to apply to each pixel of the image
 * @param pointX x value of the image that is being investigated
 * @param pointY y value of the image that is being investigated
 * @return Computed kernel value for the coordinate (pointX, pointY)
 */
static double apply_kernel_to_point(Image *img, Image *kernel, size_t pointX, size_t pointY);

/**
 * Sum all pixels of the given image
 * @param img image of all pixels that shall be summed
 * @return Summed value of all pixels of the given image
 */
static double sum_all(Image *img);

/**
 * Writes the checksum to file descriptor
 * @param fd File descriptor you want to write to
 * @param checksum Checksum that shall be written to fd
 */
static void write_checksum_to(FILE *fd, double checksum);

/**
 * Runs the default benchmark, comparable to the benchmarks in the haskell paper
 *
 * @param img Image to apply the kernel to
 * @param kernel Kernel to apply on the image
 * @param buffer Buffer image to avoid repeated allocation
 * @param numberOfIterations number of iterations to apply the kernel to the image
 */
static void run_default(Image *img, Image *kernel, Image *buffer, size_t numberOfIterations);

/**
 * Parse the args according to getopt
 *
 * @param argc number of arguments
 * @param argv arguments
 */
static void parse_args(int argc, char **argv);

/**
 * Prints Synopsis of the program
 */
static void usage();

int main(int argc, char **argv) {
    // argument parsing
    pgmname = argv[0]; // for error messages
    parse_args(argc, argv);
    int returnValue = 0;
    // parse args
    // allocate memory
    Image *image = init_image(args.width, args.height, 1.0);
    Image *kernel = get_default_kernel();
    Image *buffer = copy_shape(image);
    if (image != NULL && kernel != NULL && buffer != NULL) {
        // start benchmarking
        printf("Starting Kernel...\n");
        TIC(0);
        run_default(image, kernel, buffer, args.numberOfIterations);
        time_t seq_t = TOC(0);

        // print kernel time
        printf("Kernel time: %zu.%06zus\n", seq_t / 1000000, seq_t % 1000000);

        // print to file
        FILE *fd = fopen("2d-convolution", "w+");
        write_checksum_to(fd, sum_all(image));
        if (fd != NULL) {
            fclose(fd);
        }
    } else {
        returnValue = 1;
    }
    // free resources
    free_image(image);
    free_image(kernel);
    free_image(buffer);
    return returnValue;
}

static void parse_args(int argc, char **argv) {
    // default values according to the haskell program
    args.numberOfIterations = 4000;
    args.width = 1024;
    args.height = 1024;

    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?n:p:w:h:")) != -1) {
        switch (c) {
            case 'n':
                args.numberOfIterations = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'p':
                args.numberOfProcesses = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'w':
                args.width = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'h':
                args.height = (size_t) strtol(optarg, NULL, 10);
                break;
            case '?':
                usage();
                break;
            default:
                usage();
                break;
        }
    }
}

static void usage() {
    fprintf(stderr, "SYNOPSIS: %s [-p number_of_processes] [-w width] [-h height] [-n iterations]", pgmname);
    exit(1);
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
    for (size_t y = 0; y < img->height; ++y) {
        img->image[y] = (double *) malloc(sizeof(double) * 10);
        for (size_t x = 0; x < img->width; ++x) {
            img->image[y][x] = default_val;
        }
    }
    return img;
}

static Image *copy_shape(Image *img) {
    if (img != NULL) {
        return init_image(img->width, img->height, 0.0);
    } else {
        return NULL;
    }
}

static Image *get_default_kernel(void) {
    Image *img = init_image(5, 5, 0);
    for (int height = 0; height < img->height; ++height) {
        for (int width = 0; width < img->width; ++width) {
            if ((height + width) % 2 == 0) {
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

static void apply_kernel_to_image(Image *img, Image *kernel, Image *buffer) {
    for (size_t imgHeight = 0; imgHeight < img->height; ++imgHeight) {
        for (size_t imgWidth = 0; imgWidth < img->width; ++imgWidth) {
            buffer->image[imgHeight][imgWidth] = apply_kernel_to_point(img, kernel, imgWidth, imgHeight);
        }
    }
}

static double apply_kernel_to_point(Image *img, Image *kernel, size_t pointX, size_t pointY) {
    double val = 0.0;
    for (size_t y = 0; y < kernel->height; ++y) {
        for (size_t x = 0; x < kernel->width; ++x) {
            size_t imageIndexY = clamp(0, pointY - y + 2, img->height - 1);
            size_t imageIndexX = clamp(0, pointX - x + 2, img->width - 1);
            val +=
                    kernel->image[y][x]
                    * img->image[imageIndexY][imageIndexX];
        }
    }

    return val;
}

static double sum_all(Image *img) {
    double val = 0.0;
    for (int imgHeight = 0; imgHeight < img->height; ++imgHeight) {
        for (int imgWidth = 0; imgWidth < img->width; ++imgWidth) {
            val += img->image[imgHeight][imgWidth];
        }
    }
    return val;
}

static void write_checksum_to(FILE *fd, double checksum) {
    if (fd != NULL) {
        fprintf(fd, "%f\n", checksum);
    } else {
        fprintf(stdout, "%f\n", checksum);
    }
}

static void run_default(Image *img, Image *kernel, Image *buffer, size_t numberOfIterations) {
    for (size_t i = 0; i < numberOfIterations; i++) {

        apply_kernel_to_image(img, kernel, buffer);

        swap(img, buffer);
    }
}
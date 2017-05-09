//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>
#include <getopt.h>
#include "util.h"
#include <stdbool.h>
#include <string.h>

#define MAX_SIZE 16384
/* img pointer must be */
#define ACCESS_IMG_OFF(img, x, y, width) (img[(y)*(width)+(x)])
#define ACCESS_IMG(img, x, y, width) (img[(2+(y))*(width)+(x)+2])


#define HALF_KERNEL_WIDTH (2)
#define HALF_KERNEL_HEIGHT (2)

#define ADDITIONAL_WIDTH (HALF_KERNEL_WIDTH << 1)
#define ADDITIONAL_HEIGHT (HALF_KERNEL_HEIGHT << 1)
struct Image {
    size_t width;
    size_t height;
    double *image;
};

struct {
    bool debug;
    bool opt_image_from_file;
    bool opt_width;
    bool opt_height;
    size_t number_of_iterations;
    size_t number_of_processes;
    size_t width;
    size_t height;
    char *image_file_path;
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
static void apply_kernel_to_image(Image *restrict img, Image *buffer);

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
 * @param number_of_iterations number of iterations to apply the kernel to the image
 */
static void run_default(Image *restrict img, Image *restrict buffer, size_t number_of_iterations);

/**
 * Helper function to print the image in an easy way to read
 * @param img Image that shall be printed, line based
 */
static void print_image(Image *img);

/**
 * Parse the args according to getopt
 *
 * @param argc number of arguments
 * @param argv arguments
 */
static void parse_args(int argc, char **argv);

/**
 *
 * @param fd
 * @return
 */
static Image *read_image_from_fd(FILE *fd);

/**
 * Prints Synopsis of the program
 */
static void usage();

/**
 * Debug function to print the args given to the program
 */
static void print_args();


static inline double
apply_default_kernel_to_point(Image *restrict img, size_t pointX, size_t pointY);

static void bail_out(char *string);


int main(int argc, char **argv) {
    // argument parsing
    pgmname = argv[0]; // for error messages

    parse_args(argc, argv);
    if (args.debug) {
        print_args();
    }
    // parse args
    // allocate memory
    Image *image;
    if (args.opt_image_from_file) {
        FILE *fd = fopen(args.image_file_path, "r");
        image = read_image_from_fd(fd);
    } else {
        image = init_image(args.width, args.height, 1.0);
    }
    Image *buffer = copy_shape(image);
    // sanity check
    if (image != NULL && buffer != NULL) {
        // start benchmarking
        printf("Starting Kernel...\n");
        TIC(0);
        run_default(image, buffer, args.number_of_iterations);
        time_t seq_t = TOC(0);

        // print kernel time
        printf("Kernel time: %zu.%06zus\n", seq_t / 1000000, seq_t % 1000000);

        // print to file
        FILE *fd = fopen("../2d-convolution.res", "w+");
        write_checksum_to(fd, sum_all(image));
        if (fd != NULL) {
            fclose(fd);
        }
    } else {
        // free resources, just to be sure
        free_image(image);
        free_image(buffer);
        exit(EXIT_FAILURE);
    }
    // free resources
    free_image(image);
    free_image(buffer);

    return 0;
}

static void print_args() {
    printf("Args: width: %zu, height: %zu, iterations: %zu, processes: %zu\n", args.width, args.height,
           args.number_of_iterations, args.number_of_processes
    );
    printf("\topt_image_file: %d\n", args.opt_image_from_file);
    if (args.opt_image_from_file) {
        printf("\topt_image_file_path: %s\n", args.image_file_path);
    }
}

static void parse_args(int argc, char **argv) {
    // default values according to the haskell program
    args.number_of_iterations = 4000;
    args.width = 1024;
    args.height = 1024;
    args.number_of_processes = 1;
    args.debug = false;
    args.opt_image_from_file = false;
    args.opt_width = false;
    args.opt_height = false;

    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?dn:p:w:h:f:")) != -1) {
        switch (c) {
            case 'd':
                args.debug = true;
                break;
            case 'f':
                args.opt_image_from_file = true;
                args.image_file_path = optarg;
                break;
            case 'n':
                args.number_of_iterations = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'p':
                args.number_of_processes = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'w':
                args.opt_width = true;
                args.width = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'h':
                args.opt_height = true;
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

    if (args.opt_image_from_file && (args.opt_width || args.opt_height)) {
        usage();
    }

    if (args.height == 0 || args.width == 0 || args.number_of_processes == 0) {
        usage();
    }
}

static void usage() {
    fprintf(stderr,
            "SYNOPSIS: %s [-d] [-p number_of_processes] ([-w width] [-h height] || -f image_file_name) [-k kernel_file_name] [-n iterations]\n",
            pgmname);
    exit(1);
}


static void free_image(Image *image) {
    if (image != NULL) {
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
    img->image = (double *) malloc(sizeof(double) * (height + ADDITIONAL_HEIGHT) * (width + ADDITIONAL_WIDTH));
    for (size_t y = 0; y < img->height + ADDITIONAL_HEIGHT; ++y) {
        for (size_t x = 0; x < img->width + ADDITIONAL_WIDTH; ++x) {
            img->image[y * (width + ADDITIONAL_WIDTH) + x] = default_val;
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


static void apply_kernel_to_image(Image *restrict img, Image *buffer) {
#pragma omp parallel for num_threads(args.number_of_processes)
    for (size_t y = 0; y < img->height; ++y) {
        for (size_t x = 0; x < img->width; ++x) {
            size_t width = img->width + ADDITIONAL_WIDTH;
            ACCESS_IMG(img->image, x, y, width) = apply_default_kernel_to_point(img, x, y);
        }
    }
}

static inline double
apply_default_kernel_to_point(Image *restrict img, size_t pointX, size_t pointY) {
    size_t imgw = img->width + ADDITIONAL_WIDTH; //maybe does something ~0.5 seconds
    double *precomputedImageStart =
            img->image + (2 * imgw) + 2; //image start + two full lines + 2 offset, Does do ~2 seconds

    // compile kernel into application
    return 0.0 //variant ordered by same x
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 2, pointY - 2, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 2, pointY - 1, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 2, pointY, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 2, pointY + 1, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 2, pointY + 2, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 1, pointY - 2, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 1, pointY - 1, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 1, pointY, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 1, pointY + 1, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX - 1, pointY + 2, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX, pointY - 2, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX, pointY - 1, imgw)
           + -1 * ACCESS_IMG_OFF(precomputedImageStart, pointX, pointY, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX, pointY + 1, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX, pointY + 2, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 1, pointY - 2, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 1, pointY - 1, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 1, pointY, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 1, pointY + 1, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 1, pointY + 2, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 2, pointY - 2, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 2, pointY - 1, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 2, pointY, imgw)
           + -2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 2, pointY + 1, imgw)
           + 2 * ACCESS_IMG_OFF(precomputedImageStart, pointX + 2, pointY + 2, imgw);
}


static double sum_all(Image *img) {
    double val = 0.0;
    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            val += ACCESS_IMG_OFF(img->image, x, y, img->width + ADDITIONAL_WIDTH);
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

static void
run_default(Image *restrict img, Image *restrict buffer, size_t number_of_iterations) {
    for (size_t i = 0; i < number_of_iterations; i++) {
        apply_kernel_to_image(img, buffer);
        swap_ptr((void **) img, (void **) buffer);
    }
}

static void print_image(Image *img) {
    for (int height = 0; height < img->height; ++height) {
        for (int width = 0; width < img->width; ++width) {
            char *format = NULL;
            if (ACCESS_IMG_OFF(img->image, width, height, img->width + ADDITIONAL_WIDTH) < 0) {
                format = "%.02f\t";
            } else {
                format = " %.02f\t";
            }
            printf(format, ACCESS_IMG_OFF(img->image, width, height, img->width + ADDITIONAL_WIDTH));
        }
        printf("\n");
    }
}

static Image *read_image_from_fd(FILE *fd) {
    if (fd != NULL) {
        char buf[MAX_SIZE];
        if (fgets(buf, MAX_SIZE, fd) != NULL) { ;
            char *token = NULL;
            token = strtok(buf, " ");
            if (NULL == token) bail_out("height couldn't be read from file");
            size_t height = (size_t) strtol(token, NULL, 10);
            token = strtok(buf, " ");
            if (NULL == token) bail_out("width couldn't be read from file");
            size_t width = (size_t) strtol(token, NULL, 10);

            Image *img = init_image(width, height, 0.0);

            for (int y = 0; y < height; ++y) {
                if (NULL != fgets(buf, MAX_SIZE, fd)) {
                    token = strtok(buf, " ");
                    for (int x = 0; x < width; ++x) {
                        if (token != NULL) {
                            img->image[y + x * (y + 1)] = strtod(token, NULL);
                        } else {
                            bail_out("the file dimensions were not correct, less number of lines than given width");
                        }
                        token = strtok(NULL, " ");
                    }
                } else {
                    bail_out("the file did not have enough lines to read");
                }
            }

            return img;
        }
    } else {
        bail_out("file descriptor couldn't was closed");
    }
    return NULL;
}

static void bail_out(char *string) {
    fprintf(stderr, "Error: %s: %s", pgmname, string);
    exit(2);
}

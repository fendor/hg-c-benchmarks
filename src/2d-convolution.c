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
struct Image {
    size_t width;
    size_t height;
    double **image;
};

struct {
    bool debug;
    bool opt_image_from_file;
    bool opt_kernel_from_file;
    bool opt_width;
    bool opt_height;
    size_t number_of_iterations;
    size_t number_of_processes;
    size_t width;
    size_t height;
    char *image_file_path;
    char *kernel_file_path;
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
 * This Image is equal to the following Kernel
 *    0  1  0
 *    1 -4  0
 *    0  1  0
 *
 * @return Return laplace Kernel Image
 */
static Image *get_2d_laplace_kernel(void);


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
static void apply_kernel_to_image(Image *restrict img, Image *restrict kernel, Image *buffer);

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
static double apply_kernel_to_point(Image *restrict img, Image *restrict kernel, size_t pointX, size_t pointY);

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
static void
run_default(Image *restrict img, Image *restrict kernel, Image *restrict buffer, size_t number_of_iterations);

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

static void print_args();

static inline double smart_access(Image *img, size_t x, size_t y);

static inline double
apply_default_kernel_to_point(Image *restrict img, Image *restrict kernel, size_t pointX, size_t pointY);

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
    Image *kernel;
    if (args.opt_kernel_from_file) {
        FILE *fd = fopen(args.kernel_file_path, "r");
        kernel = read_image_from_fd(fd);
    } else {
        kernel = get_default_kernel();
    }
    Image *buffer = copy_shape(image);
    if (args.debug) {
        print_image(kernel);
    }
    // sanity check
    if (image != NULL && kernel != NULL && buffer != NULL) {
        // start benchmarking
        printf("Starting Kernel...\n");
        TIC(0);
        run_default(image, kernel, buffer, args.number_of_iterations);
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
        // free resources
        free_image(image);
        free_image(kernel);
        free_image(buffer);
        exit(EXIT_FAILURE);
    }
    // free resources
    free_image(image);
    free_image(kernel);
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
    printf("\topt_kernel_file: %d\n", args.opt_kernel_from_file);
    if (args.opt_kernel_from_file) {
        printf("\topt_image_file_path: %s\n", args.kernel_file_path);
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
    args.opt_kernel_from_file = false;
    args.opt_width = false;
    args.opt_height = false;

    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?dn:p:w:h:k:f:")) != -1) {
        switch (c) {
            case 'd':
                args.debug = true;
                break;
            case 'k':
                args.opt_kernel_from_file = true;
                args.kernel_file_path = optarg;
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
    img->image = (double **) malloc(sizeof(double *) * height);
    for (size_t y = 0; y < img->height; ++y) {
        img->image[y] = (double *) malloc(sizeof(double) * width);
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

static Image *get_2d_laplace_kernel(void) {
    Image *img = init_image(3, 3, 0);
    img->image[1][1] = -4;
    img->image[0][1] = 1;
    img->image[2][1] = 1;
    img->image[1][0] = 1;
    img->image[1][2] = 1;

    return img;
}


static void apply_kernel_to_image(Image *restrict img, Image *restrict kernel, Image *buffer) {
#pragma omp parallel for num_threads(args.number_of_processes)
    for (size_t imgHeight = 0; imgHeight < img->height; ++imgHeight) {
        for (size_t imgWidth = 0; imgWidth < img->width; ++imgWidth) {
            buffer->image[imgHeight][imgWidth] = apply_default_kernel_to_point(img, kernel, imgWidth, imgHeight);
        }
    }
}

static inline double
apply_default_kernel_to_point(Image *restrict img, Image *restrict kernel, size_t pointX, size_t pointY) {
    return 0.0
           + kernel->image[0][0] * smart_access(img, pointX - 2, pointY - 2)
           + kernel->image[0][1] * smart_access(img, pointX - 1, pointY - 2)
           + kernel->image[0][2] * smart_access(img, pointX, pointY - 2)
           + kernel->image[0][3] * smart_access(img, pointX + 1, pointY - 2)
           + kernel->image[0][4] * smart_access(img, pointX + 2, pointY - 2)
           + kernel->image[1][0] * smart_access(img, pointX - 2, pointY - 1)
           + kernel->image[1][1] * smart_access(img, pointX - 1, pointY - 1)
           + kernel->image[1][2] * smart_access(img, pointX, pointY - 1)
           + kernel->image[1][3] * smart_access(img, pointX + 1, pointY - 1)
           + kernel->image[1][4] * smart_access(img, pointX + 2, pointY - 1)
           + kernel->image[2][0] * smart_access(img, pointX - 2, pointY)
           + kernel->image[2][1] * smart_access(img, pointX - 1, pointY)
           + kernel->image[2][2] * smart_access(img, pointX, pointY)
           + kernel->image[2][3] * smart_access(img, pointX + 1, pointY)
           + kernel->image[2][4] * smart_access(img, pointX + 2, pointY)
           + kernel->image[3][0] * smart_access(img, pointX - 2, pointY + 1)
           + kernel->image[3][1] * smart_access(img, pointX - 1, pointY + 1)
           + kernel->image[3][2] * smart_access(img, pointX, pointY + 1)
           + kernel->image[3][3] * smart_access(img, pointX + 1, pointY + 1)
           + kernel->image[3][4] * smart_access(img, pointX + 2, pointY + 1)
           + kernel->image[4][0] * smart_access(img, pointX - 2, pointY + 2)
           + kernel->image[4][1] * smart_access(img, pointX - 1, pointY + 2)
           + kernel->image[4][2] * smart_access(img, pointX, pointY + 2)
           + kernel->image[4][3] * smart_access(img, pointX + 1, pointY + 2)
           + kernel->image[4][4] * smart_access(img, pointX + 2, pointY + 2);

}

static inline double smart_access(Image *img, size_t x, size_t y) {
    return img->image[clamp(0, y, img->height - 1)][clamp(0, x, img->width - 1)];
}


static inline double apply_kernel_to_point(Image *restrict img, Image *restrict kernel, size_t pointX, size_t pointY) {
    double val = 0.0;
    for (size_t y = 0; y < kernel->height; ++y) {
        for (size_t x = 0; x < kernel->width; ++x) {
            size_t imageIndexY = clamp(0, pointY + y - kernel->height / 2, img->height - 1);
            size_t imageIndexX = clamp(0, pointX + x - kernel->width / 2, img->width - 1);
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

static void
run_default(Image *restrict img, Image *restrict kernel, Image *restrict buffer, size_t number_of_iterations) {
    for (size_t i = 0; i < number_of_iterations; i++) {
        apply_kernel_to_image(img, kernel, buffer);
        swap_ptr((void **) img, (void **) buffer);
    }
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
                            img->image[y][x] = strtod(token, NULL);
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

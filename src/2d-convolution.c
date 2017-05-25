//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>
#include "util.h"
#include "convolution-util.h"


/**
 * Runs the default benchmark, comparable to the benchmarks in the haskell paper
 *
 * @param img Image to apply the kernel to
 * @param kernel Kernel to apply on the image
 * @param buffer Buffer image to avoid repeated allocation
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 */
static void
run_default(Image **restrict img, const Image *restrict kernel, const Args *args, Image **restrict buffer);

/**
 * Runs the default benchmark, comparable to the benchmarks in the haskell paper
 *
 * @param img Image to apply the kernel to
 * @param kernel Kernel to apply on the image
 * @param buffer Buffer image to avoid repeated allocation
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 */
static void
run(Image **restrict img, const Image *restrict kernel, const Args *args, Image **restrict buffer);


/**
 * Apply a given kernel on every pixel of an image.
 * Computed values are written to the buffer.
 * Values that are outside of the image are set to the edge values
 *
 * @param img Image to which the kernel is applied
 * @param kernel Kernel to apply to each pixel of the image
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 * @param buffer Output buffer
 */
static void apply_kernel_to_image(Image *restrict img, const Image *restrict kernel, const Args *args, Image *buffer);

/**
 * Apply a given kernel of the size 5x5 on every pixel of an image.
 * Computed values are written to the buffer.
 * Values that are outside of the image are set to the edge values
 *
 * @param img Image to which the kernel is applied
 * @param kernel Kernel to apply to each pixel of the image, must be 5x5
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 * @param buffer Output buffer
 */
static void
apply_default_kernel_to_image(Image *restrict img, const Image *restrict kernel, const Args *args, Image *buffer);

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
static double apply_kernel_to_point(Image *restrict img, const Image *restrict kernel, int pointX, int pointY);

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
static inline double
apply_default_kernel_to_point(Image *restrict img, const Image *restrict kernel, int pointX, int pointY);

/**
 * Create image based on the arguments
 * @param args Program arguments
 * @return Created Image
 */
static Image *create_image(Args *args);

/**
 * Create kernel based on the arguments
 * @param args Program arguments
 * @return Created Kernel
 */
static Image *create_kernel(Args *args);

/**
 * Free all allocated resources
 * @param image Image to free
 * @param kernel kernel to free
 * @param buffer buffer to free
 * @param backup backup image to free
 * @param args arguments to free
 */
static void free_resources(Image *image, Image *kernel, Image *buffer, Image *backup, Args *args);

/**
 * Benchmark how long it takes to apply the kernel to an given image.
 *
 * @param image Image to apply the kernel to
 * @param kernel Kernel to apply to the image
 * @param args Arguments like iterations, number of used processors
 * @param buffer Buffer to write the output to, must have same extent as the image
 * @return time it took to apply the kernel
 */
static time_t benchmark(Image **image, const Image *kernel, const Args *args, Image **buffer);

/**
 * Entry point of the program
 *
 * @param argc Number of arguments
 * @param argv String array of arguments
 * @return non - zero exit code indicates error.
 */
int main(int argc, char **argv) {
    // argument parsing
    pgmname = argv[0]; // for error messages
    Args *args = parse_args(argc, argv);
    // parse args
    // allocate memory
    Image *image = create_image(args);
    Image *kernel = create_kernel(args);
    Image *buffer = copy_shape(image);
    Image *backup = copy_shape(image);
    // sanity check
    if (image != NULL && kernel != NULL && buffer != NULL) {
        if (copy_image(image, backup) < 0) {
            bail_out("Could not backup image");
        }
        // open benchmark files
        FILE *res = fopen("../2d-convolution.time.res", "a+");
        FILE *check = fopen("../2d-convolution.res", "w+");
        if (res == NULL || check == NULL) {
            bail_out("Could not open benchmark output files");
        }
        // start benchmarking
        for (int i = 0; i < 10; ++i) {
            if (copy_image(backup, image) < 0) {
                bail_out("Could not restore image, something must have been changed");
            }
            time_t seq_t = benchmark(&image, kernel, args, &buffer);

            append_csv(res, image, args, seq_t);
            write_checksum_to(check, sum_all(image));
        }
        fflush(check);
        fflush(res);
        fclose(check);
        fclose(res);

    } else {
        // free resources
        if (args->debug) {
            fprintf(stderr, "Memory could not be allocated\n");
        }
        free_resources(image, kernel, buffer, backup, args);
        return EXIT_FAILURE;
    }
    // free resources
    free_resources(image, kernel, buffer, backup, args);
    return 0;
}

time_t benchmark(Image **image, const Image *kernel, const Args *args, Image **buffer) {
    time_t seq_t;
    printf("Starting Kernel...\n");
    // start the clock
    if (kernel->width == 5 && kernel->height == 5) {
        TIC(0);
        run_default(image, kernel, args, buffer);
        seq_t = TOC(0); // stop the clock
    } else {
        TIC(0);
        run(image, kernel, args, buffer);
        seq_t = TOC(0); // stop the clock
    }
    // print kernel time
    printf("Kernel time: %zu.%06zus\n", seq_t / 1000000, seq_t % 1000000);
    return seq_t;
}

static void free_resources(Image *image, Image *kernel, Image *buffer, Image *backup, Args *args) {
    free_image(image);
    free_image(kernel);
    free_image(buffer);
    free_image(backup);
    free_args(args);
}

static Image *create_kernel(Args *args) {
    Image *kernel;
    if (args->opt_kernel_from_file) {
        FILE *fd = fopen(args->kernel_file_path, "r");
        kernel = read_image_from_fd(fd, args);
    } else {
        kernel = get_default_kernel();
    }
    return kernel;
}

static Image *create_image(Args *args) {
    Image *image;
    if (args->opt_image_from_file) {
        FILE *fd = fopen(args->image_file_path, "r");
        image = read_image_from_fd(fd, args);
    } else {
        image = init_image(args->width, args->height, 1.0);
    }
    return image;
}

static void
apply_default_kernel_to_image(Image *restrict img, const Image *restrict kernel, const Args *args, Image *buffer) {
#pragma omp parallel for num_threads(args->number_of_processes)
    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            buffer->image[y][x] = apply_default_kernel_to_point(img, kernel, x, y);
        }
    }
}

static void apply_kernel_to_image(Image *restrict img, const Image *restrict kernel, const Args *args, Image *buffer) {
#pragma omp parallel for num_threads(args->number_of_processes)
    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            buffer->image[y][x] = apply_kernel_to_point(img, kernel, x, y);
        }
    }
}

static void
run_default(Image **restrict img, const Image *restrict kernel, const Args *args, Image **restrict buffer) {
    for (int i = 0; i < args->number_of_iterations; i++) {
        apply_default_kernel_to_image(*img, kernel, args, *buffer);

        swap_ptr(img, buffer, Image*);
    }
}

static void
run(Image **restrict img, const Image *restrict kernel, const Args *args, Image **restrict buffer) {
    for (int i = 0; i < args->number_of_iterations; i++) {
        apply_kernel_to_image(*img, kernel, args, *buffer);

        swap_ptr(img, buffer, Image*);
    }
}


static inline double
apply_default_kernel_to_point(Image *restrict img, const Image *restrict kernel, int pointX, int pointY) {
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


static inline double apply_kernel_to_point(Image *restrict img, const Image *restrict kernel, int pointX, int pointY) {
    double val = 0.0;
    for (int y = 0; y < kernel->height; ++y) {
        for (int x = 0; x < kernel->width; ++x) {
            int imageIndexY = clamp(0, pointY + y - kernel->height / 2, img->height - 1);
            int imageIndexX = clamp(0, pointX + x - kernel->width / 2, img->width - 1);
            val +=
                    kernel->image[y][x]
                    * img->image[imageIndexY][imageIndexX];
        }
    }

    return val;
}
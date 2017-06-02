//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>
#include "util.h"
#include "convolution-util.h"

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
 * Benchmark how long it takes to apply the kernel to an given image.
 *
 * @param image Image to apply the kernel to
 * @param kernel Kernel to apply to the image
 * @param args Arguments like iterations, number of used processors
 * @param buffer Buffer to write the output to, must have same extent as the image
 * @return time it took to apply the kernel
 */
static time_t benchmark(ImageWithPadding **image, const Image *kernel, const Args *args, ImageWithPadding **buffer);

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
static double
apply_kernel_to_padded_point(ImageWithPadding *padded_img, const Image *restrict kernel, int pointX, int pointY);

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
apply_kernel_to_padded_image(ImageWithPadding *restrict img, const Image *restrict kernel, const Args *args,
                             ImageWithPadding *buffer);

/**
 * Runs the benchmark, comparable to the benchmarks in the haskell paper
 *
 * @param img Image to apply the kernel to
 * @param kernel Kernel to apply on the image
 * @param buffer Buffer image to avoid repeated allocation
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 */
static void run_on_padded_image(ImageWithPadding **padded_img, const Image *restrict kernel, const Args *args,
                                ImageWithPadding **buffer);

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
    if (kernel == NULL || image == NULL) {
        free_image(image);
        free_image(kernel);
        free_args(args);
        bail_out("kernel or image could not be created");
    }
    ImageWithPadding *padded_img = add_padding(image, kernel->width / 2);
    ImageWithPadding *padded_buffer = add_padding(image, kernel->width / 2);
    ImageWithPadding *backup = add_padding(image, kernel->width / 2);

    free_image(image);
    // sanity check
    if (padded_img != NULL && padded_buffer != NULL && backup != NULL) {
        if (copy_padded_image(padded_img, backup) < 0) {
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
            if (copy_padded_image(backup, padded_img) < 0) {
                bail_out("Could not restore image, something must have been changed");
            }
            time_t seq_t = benchmark(&padded_img, kernel, args, &padded_buffer);

            append_csv(res, image, args, seq_t);
            // TODO: this is not optimal
            Image *img = remove_padding(padded_img);
            write_checksum_to(check, sum_all(img));
            free_image(img);
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
        free_image(kernel);
        free_padded_image(padded_img);
        free_padded_image(padded_buffer);
        free_padded_image(backup);
        return EXIT_FAILURE;
    }
    // free resources
    free_image(kernel);
    free_padded_image(padded_img);
    free_padded_image(padded_buffer);
    free_padded_image(backup);
    return 0;
}

time_t benchmark(ImageWithPadding **image, const Image *kernel, const Args *args, ImageWithPadding **buffer) {
    time_t seq_t;
    printf("Starting Kernel...\n");
    // start the clock
    TIC(0);
    run_on_padded_image(image, kernel, args, buffer);
    seq_t = TOC(0); // stop the clock

    // print kernel time
    printf("Kernel time: %zu.%06zus\n", seq_t / 1000000, seq_t % 1000000);
    return seq_t;
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

static void run_on_padded_image(ImageWithPadding **padded_img, const Image *restrict kernel, const Args *args,
                                ImageWithPadding **buffer) {
    for (int i = 0; i < args->number_of_iterations; i++) {
        apply_kernel_to_padded_image(*padded_img, kernel, args, *buffer);
        update_borders(*buffer);
        swap_ptr(padded_img, buffer, ImageWithPadding*);
    }
}


static void
apply_kernel_to_padded_image(ImageWithPadding *restrict padded_img, const Image *restrict kernel, const Args *args,
                             ImageWithPadding *buffer) {
#pragma omp parallel for num_threads(args->number_of_processes)
    for (int y = 0; y < padded_img->inner_height; ++y) {
        for (int x = 0; x < padded_img->inner_width; ++x) {
            ACCESS_IMAGE(buffer, x, y) = apply_kernel_to_padded_point(padded_img, kernel, x, y);
        }
    }
}

static double
apply_kernel_to_padded_point(ImageWithPadding *padded_img, const Image *restrict kernel, int pointX, int pointY) {
    double val = 0.0;
    for (int y = 0; y < kernel->height; ++y) {
        for (int x = 0; x < kernel->width; ++x) {

            int newX = pointX - padded_img->padding + x;
            int newY = pointY - padded_img->padding + y;
            val += kernel->image[y][x] *
                   ACCESS_IMAGE(padded_img, newX, newY);
        }
    }
    return val;
}
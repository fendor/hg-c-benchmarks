//
// Created by baldr on 7/13/17.
//

#include "convolution-run.h"

time_t benchmark(ImageWithPadding **image, Image *kernel, Args *args, ImageWithPadding **buffer) {
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

Image *create_kernel(Args *args) {
    Image *kernel;
    if (args->opt_kernel_from_file) {
        FILE *fd = fopen(args->kernel_file_path, "r");
        kernel = read_image_from_fd(fd, args);
    } else {
        kernel = get_default_kernel();
    }
    return kernel;
}

Image *create_image(Args *args) {
    Image *image;
    if (args->opt_image_from_file) {
        FILE *fd = fopen(args->image_file_path, "r");
        image = read_image_from_fd(fd, args);
    } else {
        image = init_image(args->width, args->height, 1.0);
    }
    return image;
}

void // __attribute__((noinline))
run_on_padded_image(ImageWithPadding **padded_img, Image *kernel, Args *args,
                    ImageWithPadding **buffer) {
    for (int i = 0; i < args->number_of_iterations; i++) {
        apply_kernel_to_padded_image(*padded_img, kernel, args, *buffer);
        update_borders(*buffer);
        swap_ptr(padded_img, buffer, ImageWithPadding*);
    }
}


void // __attribute__((noinline))
apply_kernel_to_padded_image(ImageWithPadding *padded_img, Image *kernel, Args *args,
                             ImageWithPadding *buffer) {
#pragma omp parallel for num_threads(args->number_of_processes)
    for (int y = 0; y < padded_img->inner_height; ++y) {
// #pragma omp simd
        for (int x = 0; x < padded_img->inner_width; ++x) {
            ACCESS_IMAGE(buffer, x, y) = apply_kernel_to_padded_point(padded_img, kernel, x, y);
        }
    }
}

double // __attribute__((noinline))
apply_kernel_to_padded_point(ImageWithPadding *padded_img, Image *kernel, int pointX, int pointY) {
    double val = 0.0;


    int newY = pointY - padded_img->padding;
// #pragma omp simd
    for (int y = 0; y < kernel->height; ++y) {
        int newX = pointX - padded_img->padding;
#pragma omp simd
        for (int x = 0; x < kernel->width; ++x) {
            val += kernel->image[y][x] * ACCESS_IMAGE(padded_img, newX, newY);
            ++newX;
        }
        ++newY;
    }
    return val;
}
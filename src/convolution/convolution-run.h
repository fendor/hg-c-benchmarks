//
// Created by baldr on 7/13/17.
//

#ifndef HG_C_BENCHMARKS_CONVOLUTION_RUN_H
#define HG_C_BENCHMARKS_CONVOLUTION_RUN_H

#include "convolution-util.h"

/**
 * Create image based on the arguments
 * @param args Program arguments
 * @return Created Image
 */
Image *create_image(Args *args);

/**
 * Create kernel based on the arguments
 * @param args Program arguments
 * @return Created Kernel
 */
Image *create_kernel(Args *args);

/**
 * Benchmark how long it takes to apply the kernel to an given image.
 *
 * @param image Image to apply the kernel to
 * @param kernel Kernel to apply to the image
 * @param args Arguments like iterations, number of used processors
 * @param buffer Buffer to write the output to, must have same extent as the image
 * @return time it took to apply the kernel
 */
time_t benchmark(ImageWithPadding **image, Image *kernel, Args *args, ImageWithPadding **buffer);

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
double
apply_kernel_to_padded_point(ImageWithPadding *padded_img, Image *kernel, int pointX, int pointY);

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
void
apply_kernel_to_padded_image(ImageWithPadding *img, Image *kernel, Args *args,
                             ImageWithPadding *buffer);

/**
 * Runs the benchmark, comparable to the benchmarks in the haskell paper
 *
 * @param img Image to apply the kernel to
 * @param kernel Kernel to apply on the image
 * @param buffer Buffer image to avoid repeated allocation
 * @param args Arguments to the program, number of iterations and used processes are used for computation
 */
void run_on_padded_image(ImageWithPadding **padded_img, Image *kernel, Args *args,
                         ImageWithPadding **buffer);


#endif //HG_C_BENCHMARKS_CONVOLUTION_RUN_H

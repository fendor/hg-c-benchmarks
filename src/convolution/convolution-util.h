//
// Created by baldr on 5/23/17.
//

#ifndef HG_C_BENCHMARKS_CONVOLUTION_UTIL_H
#define HG_C_BENCHMARKS_CONVOLUTION_UTIL_H

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include "../util/util.h"

#define ACCESS_IMAGE(img, x, y) (img->image[(y) + (img->padding)][(x) + (img->padding)])
#define ACCESS_FIELD(img, x, y) (img->image[y][x])

struct arguments {
    bool debug;
    bool opt_image_from_file;
    bool opt_kernel_from_file;
    bool opt_width;
    bool opt_height;
    int number_of_iterations;
    int number_of_processes;
    int width;
    int height;
    char *image_file_path;
    char *kernel_file_path;
};

struct Image {
    int width;
    int height;
    double **image;
};

struct ImageWithPadding {
    int padding;
    int width;
    int height;
    int inner_width;
    int inner_height;
    double **image;
};
/**
 * Typedef of the arguments struct
 */
typedef struct arguments Args;

/**
 * Typedef for easier usage
 */
typedef struct Image Image;

/**
 * Typedef for easier usage of the padded image
 */
typedef struct ImageWithPadding ImageWithPadding;

/**
 * Name of the Program.
 * Must be set in the main before calling any other function.
 */
char *pgmname;

/**
 * Helper function to print the image in an easy way to read
 * @param img Image that shall be printed, line based
 */
void print_image(Image *img);

/**
 * Parse the args according to getopt
 *
 * @param argc number of arguments
 * @param argv arguments
 */
Args *parse_args(int argc, char **argv);

/**
 * Read an image from file.
 * Must start with the following form:
 * <width> <height>
 * These values must match the true extent of the image.
 * Values are separated by space. If one of the values can not be parsed, the function terminates.
 *
 * @param fd File to read the matrix from
 * @return Image read from a file
 */
Image *read_image_from_fd(FILE *fd, Args *args);

/**
 * Prints Synopsis of the program
 */
void usage();

/**
 * Print the argument struct
 * @param args Arguments to print
 */
void print_args(Args *args);

/**
 * Utility function to copy the image from one image to another.
 * The images must have the same extent, otherwise an error code is returned.
 *
 * Zero indicates a success.
 * -1 means that one of the images is NULL and -2 means that the extent do not match
 *
 * @param from Image to copy from
 * @param to Image to copy to
 * @return Error code, non-zero indicates error
 */
int copy_image(Image *from, Image *to);

/**
 * Bail out of the program
 * @param string Custom message, may be NULL
 */
void bail_out(char *string);

/**
 * Append to a file the output of the benchmark.
 * The File mustn't be NULL
 * @param fd File to append the results to
 * @param img Image containing matching width and height values
 * @param args Arguments for number of processes and how many times the benchmarks has been iterated
 * @param time Time it took to execute the benchmark
 */
void append_csv(FILE *fd, ImageWithPadding *img, Args *args, time_t time);

/**
 * Sum all pixels of the given image, represents a checksum for comparison
 * @param img image of all pixels that shall be summed
 * @return Summed value of all pixels of the given image
 */
double get_checksum(Image *img);

/**
 * Writes the checksum to file descriptor
 * @param fd File descriptor you want to write to
 * @param checksum Checksum that shall be written to fd
 */
void write_checksum_to(FILE *fd, double checksum);

/**
 * Initialize an image
 *
 * @param width width of the image
 * @param height height of the image
 * @param default_value set to every value in the image
 * @return image pointer with the set value and the respective sizes
 */
Image *init_image(int width, int height, double default_value);

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
Image *get_default_kernel(void);

/**
 * This Image is equal to the following Kernel
 *    0  1  0
 *    1 -4  1
 *    0  1  0
 *
 * @return Return laplace Kernel Image
 */
Image *get_2d_laplace_kernel(void);


/**
 * Free the allocated resources of an image pointer including the image struct itself
 * @param image Image which shall be freed
 */
void free_image(Image *image);

/**
 * Copy the dimensions of the given image.
 *
 * @param img Image that provides the dimensions for the new image
 * @return Image with the same dimensions as the old image
 */
Image *copy_shape(Image *img);

/**
 * Accesses the array of the image smartly.
 * @param img Image to read the array from
 * @param x Is clamped between 0 and the width of the image
 * @param y Is clamped between 0 and the height of the image
 * @return Value after the indices have been clamped
 */
double smart_access(Image *img, int x, int y);

/**
 * Free the arguments struct
 * @param args struct to free
 */
void free_args(Args *args);

/**
 * Add padding to an image.
 * This function is required if one wants to use a high performance approach to the 2d-convolution problem
 * @param img Image to apply the padding
 * @param padding Size of the padding for the new padded image
 * @return New padded image, must be freed with free_padded_image(...) after usage
 */
ImageWithPadding *add_padding(Image *img, int padding);

/**
 * Remove the padding of an image and return just the image data.
 * it should hold: remove_padding(add_padding(img,2)) == img
 * where (==) is the equality operator for all values
 * @param img Padded image to remove the paddings from
 * @return New image that only contains image data.
 */
Image *remove_padding(ImageWithPadding *img);

/**
 * Update the out of bounds padding of the image to match their actual borders.
 * This function is helpful, if you apply kernels more than once
 * @param padded_img Image to update the borders of
 */
void update_borders(ImageWithPadding *padded_img);

/**
 * Helper function to print an image in an human readable way
 * @param padded_img Image to print
 */
void print_padded_image(ImageWithPadding *padded_img);

/**
 * Free the resources allocated by this image
 * @param padded_img Padded image to free
 */
void free_padded_image(ImageWithPadding *padded_img);

/**
 * Copy the padded image from a to b
 * This function returns zero on success, -1 if one of the parameters is NULL and -2 if the extents do not match
 * @param padded_from Source image
 * @param padded_to Target image
 * @return zero on success, non-zero on failure
 */
int copy_padded_image(ImageWithPadding *padded_from, ImageWithPadding *padded_to);

/**
 * Initializes a new image with a padded border. All values are initialized to 0
 * Only function with @href free_padded_image that knows about the internal representation
 *
 * @param inner_width Width of the actual image
 * @param inner_height Height of the actual image
 * @param padding Size of the padding that should be added to the image
 * @return Image with a padded border. This can improve the performance of the 2d-convolution problem
 */
ImageWithPadding *init_padded_image(int inner_width, int inner_height, int padding);

#endif //HG_C_BENCHMARKS_CONVOLUTION_UTIL_H

//
// Created by baldr on 5/2/17.
//

#include<stdio.h>
#include<stdlib.h>
#include "convolution/convolution-util.h"
#include "convolution/convolution-run.h"

/**
 * Free all the resources
 */
void free_resources(Args *args, Image *kernel, ImageWithPadding *padded_img,
                    ImageWithPadding *padded_buffer, ImageWithPadding *backup);

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
            free_resources(args, kernel, padded_img, padded_buffer, backup);
            bail_out("Could not backup image");
        }
        // open benchmark files
        // TODO: make this definable by argument
        FILE *res = fopen("../2d-convolution.time.res", "a+");
        FILE *check = fopen("../2d-convolution.res", "w+");
        if (res == NULL || check == NULL) {
            free_resources(args, kernel, padded_img, padded_buffer, backup);
            bail_out("Could not open benchmark output files");
        }
        // start benchmarking
        // TODO: make this definable by argument
        for (int i = 0; i < 10; ++i) {
            if (copy_padded_image(backup, padded_img) < 0) {
                free_resources(args, kernel, padded_img, padded_buffer, backup);
                bail_out("Could not restore image, something must have been changed");
            }
            time_t seq_t = benchmark(&padded_img, kernel, args, &padded_buffer);

            append_csv(res, padded_img, args, seq_t);
            // TODO: this is not optimal
            // could be reused
            Image *img = remove_padding(padded_img);
            write_checksum_to(check, get_checksum(img));
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
        free_resources(args, kernel, padded_img, padded_buffer, backup);
        return EXIT_FAILURE;
    }
    // free resources
    free_resources(args, kernel, padded_img, padded_buffer, backup);
    return 0;
}

void free_resources(Args *args, Image *kernel, ImageWithPadding *padded_img,
                    ImageWithPadding *padded_buffer, ImageWithPadding *backup) {
    free_image(kernel);
    free_padded_image(padded_img);
    free_padded_image(padded_buffer);
    free_padded_image(backup);
    free_args(args);
}

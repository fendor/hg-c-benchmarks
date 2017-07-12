//
// Created by baldr on 5/23/17.
//
#include <gtest/gtest.h>
#include "../src/2d-convolution.c"

TEST(run_on_image, run_once) {
    /* initialize data */
    Image *img = init_image(3, 3, 1);
    /* {{ -1.5,  0,  -1},
     * {     9, -2,   3},
     * {     2,  0,   5}};
     * */
    img->image[0][0] = -1.5;
    img->image[0][1] = 0;
    img->image[0][2] = -1;
    img->image[1][0] = 9;
    img->image[1][1] = -2;
    img->image[1][2] = 3;
    img->image[2][0] = 2;
    img->image[2][1] = 0;
    img->image[2][2] = 5;

    Args args = {
            .number_of_iterations = 1,
            .opt_height= true,
            .height = 3,
            .width = 3,
            .opt_width = true,
            .number_of_processes = 1,
    };

    ImageWithPadding *padded = add_padding(img);
    free_image(img);
    ImageWithPadding *buffer = copy_padded_image(padded);
    Image *laplace = get_2d_laplace_kernel();
    run_on_padded_image(&padded, laplace, args, &buffer);
    Image *result = remove_padding(padded);

    free_padded_image(padded);
    free_padded_image(buffer);
    free_image(result);
    free_image(laplace);
}


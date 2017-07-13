//
// Created by baldr on 5/23/17.
//
#include <gtest/gtest.h>
#include "../src/convolution/convolution-util.h"
#include "../src/convolution/convolution-run.h"
#include "../src/convolution/convolution-run.c"

TEST(run_on_image, run_laplace_neutralizes_image) {
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
            false,
            false,
            false,
            true,
            true,
            15,
            1,
            3,
            3,
            NULL,
            NULL
    };

    /* prepare kernel */
    ImageWithPadding *padded = add_padding(img, 1);
    ImageWithPadding *buffer = init_padded_image(3, 3, 1);
    copy_padded_image(padded, buffer);
    Image *laplace = init_image(3, 3, 0);
    laplace->image[1][1] = -1;
    laplace->image[0][1] = 0.25;
    laplace->image[2][1] = 0.25;
    laplace->image[1][0] = 0.25;
    laplace->image[1][2] = 0.25;
    /* actual test */
    /* sanity test */
    ASSERT_EQ(14.5, get_checksum(img));
    /* run 15 times */
    run_on_padded_image(&padded, laplace, &args, &buffer);
    Image *result = remove_padding(padded);
    /* check result */
    ASSERT_EQ(0, get_checksum(result));
    /* free resources */
    free_padded_image(padded);
    free_padded_image(buffer);
    free_image(result);
    free_image(laplace);
    free_image(img);
}

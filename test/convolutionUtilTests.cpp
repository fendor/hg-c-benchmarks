//
// Created by baldr on 5/23/17.
//

#include <gtest/gtest.h>
#include "../src/convolution-util.h"
#include "../src/convolution-util.c"

#define ASSERT_ARRAY_EQ(a1, a2, len) do {for (int i=0; i<len; i++) { ASSERT_EQ(a1[i],a2[i]); }} while (0)


TEST(init_image, init_image) {
    Image *img = init_image(5, 5, 0);
    double arr[5][5] = {{0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0}};

    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            ASSERT_EQ(img->image[y][x], arr[y][x]);
        }
    }
    ASSERT_EQ(5, img->width);
    ASSERT_EQ(5, img->height);

    free_image(img);
}

TEST(init_image, init_image_with_negative_number) {
    Image *img = init_image(5, 5, -5);
    double arr[5][5] = {{-5, -5, -5, -5, -5},
                        {-5, -5, -5, -5, -5},
                        {-5, -5, -5, -5, -5},
                        {-5, -5, -5, -5, -5},
                        {-5, -5, -5, -5, -5}};

    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            ASSERT_EQ(img->image[y][x], arr[y][x]);
        }
    }
    ASSERT_EQ(5, img->width);
    ASSERT_EQ(5, img->height);

    free_image(img);
}

TEST(init_padded_image, init_image_padded) {
    ImageWithPadding *padding = init_padded_image(3, 3, 2);
    double arr[5][5] = {{0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0}};

    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            ASSERT_EQ(padding->image[y][x], arr[y][x]);
        }
    }
    ASSERT_EQ(7, padding->width);
    ASSERT_EQ(7, padding->height);
    ASSERT_EQ(3, padding->inner_width);
    ASSERT_EQ(3, padding->inner_height);

    free_padded_image(padding);
}

TEST(init_padded_image, add_padding) {
    Image *img = init_image(3, 3, 1);
    ImageWithPadding *padding = add_padding(img, 2);
    double arr[7][7] = {{1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1, 1, 1}};

    for (int x = 0; x < 7; ++x) {
        for (int y = 0; y < 7; ++y) {
            ASSERT_EQ(padding->image[y][x], arr[y][x]);
        }
    }
    ASSERT_EQ(7, padding->width);
    ASSERT_EQ(7, padding->height);
    ASSERT_EQ(3, padding->inner_width);
    ASSERT_EQ(3, padding->inner_height);

    free_padded_image(padding);
    free_image(img);
}

TEST(init_padded_image, add_padding_complicated_image) {
    Image *img = init_image(3, 3, 1);
    /* {{-1.5, 0,  -1},
     * {9,    -2, 3},
     * {2,    0,  5}};
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

    ImageWithPadding *padding = add_padding(img, 2);
    double arr[7][7] = {{-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                        {-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                        {-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                        {9,    9,    9,    -2, 3,  3,  3},
                        {2,    2,    2,    0,  5,  5,  5},
                        {2,    2,    2,    0,  5,  5,  5},
                        {2,    2,    2,    0,  5,  5,  5}};

    for (int x = 0; x < 7; ++x) {
        for (int y = 0; y < 7; ++y) {
            ASSERT_EQ(padding->image[y][x], arr[y][x]);
        }
    }
    ASSERT_EQ(7, padding->width);
    ASSERT_EQ(7, padding->height);
    ASSERT_EQ(3, padding->inner_width);
    ASSERT_EQ(3, padding->inner_height);

    free_padded_image(padding);
    free_image(img);
}

TEST(to_image, remove_complicated_padding) {
    /* initialize data */
    double expected[3][3] = {{-1.5, 0,  -1},
                             {9,    -2, 3},
                             {2,    0,  5}};

    ImageWithPadding *padding = init_padded_image(3, 3, 2);
    double padded_image[7][7] = {{-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                                 {-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                                 {-1.5, -1.5, -1.5, 0,  -1, -1, -1},
                                 {9,    9,    9,    -2, 3,  3,  3},
                                 {2,    2,    2,    0,  5,  5,  5},
                                 {2,    2,    2,    0,  5,  5,  5},
                                 {2,    2,    2,    0,  5,  5,  5}};
    /* copy data */
    for (int x = 0; x < 7; ++x) {
        for (int y = 0; y < 7; ++y) {
            ACCESS_FIELD(padding, x, y) = padded_image[y][x];
        }
    }

    /* actual test */
    Image *img = remove_padding(padding);
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            ASSERT_EQ(expected[y][x], img->image[y][x]);
        }
    }
    ASSERT_EQ(7, padding->width);
    ASSERT_EQ(7, padding->height);
    ASSERT_EQ(3, padding->inner_width);
    ASSERT_EQ(3, padding->inner_height);
    ASSERT_EQ(3, img->width);
    ASSERT_EQ(3, img->height);

    free_padded_image(padding);
    free_image(img);
}

TEST(get_checksum, calculate_simple_checksum) {
    Image *img = init_image(5, 5, -5);

    ASSERT_EQ(-5 * 5 * 5, get_checksum(img));

    free_image(img);
}
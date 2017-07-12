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
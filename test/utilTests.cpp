//
// Created by baldr on 5/23/17.
//
#include <gtest/gtest.h>
#include "../src/util/util.c"
#include "../src/util/util.h"


TEST(clamper, clamp) {
    ASSERT_EQ(0, clamp(0, -2, 6));
    ASSERT_EQ(6, clamp(0, 8, 6));
    ASSERT_EQ(6, clamp(0, 6, 6));
    ASSERT_EQ(0, clamp(0, 0, 6));
    ASSERT_EQ(1, clamp(0, 1, 6));
    ASSERT_EQ(5, clamp(0, 5, 6));
    ASSERT_EQ(-7, clamp(-10, -7, -5));
}

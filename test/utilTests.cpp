//
// Created by baldr on 5/23/17.
//
#include <gtest/gtest.h>
#include "../src/util.c"
#include "../src/util.h"

#define ASSERT_ARRAYEQ(a1, a2, len) do {for (int i=0; i<len; i++) { ASSERT_EQ(a1[i],a2[i]); }} while (0)

TEST(clamper, clamp) {
    ASSERT_EQ(0, clamp(0, -2, 6));
    ASSERT_EQ(6, clamp(0, 8, 6));
    ASSERT_EQ(6, clamp(0, 6, 6));
    ASSERT_EQ(0, clamp(0, 0, 6));
    ASSERT_EQ(1, clamp(0, 1, 6));
    ASSERT_EQ(5, clamp(0, 5, 6));
    ASSERT_EQ(-7, clamp(-10, -7, -5));
}
/*
TEST(swap_ptr, swap_array) {
    int control1[] = {1, 2, 3, 4};
    int control2[] = {6, 5, 4, 3};

    int initA[] = {1, 2, 3, 4};
    int initB[] = {6, 5, 4, 3};

    int *a = initA;
    int *b = initB;
    int **i;
    int **j;
    i = &a;
    j = &b;
    swap_ptr(i, j, int*);
    ASSERT_ARRAYEQ(control1, initB, 4);
    ASSERT_ARRAYEQ(control2, initA, 4);
}

TEST(swap_ptr, swap_array_and_increment) {
    int a[] = {1, 2, 3, 4};
    int b[] = {6, 5, 4, 3};

    swap_ptr(&a, &b, int*);
    b[0] += 2;
    ASSERT_EQ(3, b[0]);
    ASSERT_EQ(6, a[0]);
}*/
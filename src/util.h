//
// Created by baldr on 5/2/17.
//
#ifndef HG_C_BENCHMARKS_UTIL_H
#define HG_C_BENCHMARKS_UTIL_H

#include <sys/time.h>
#include <stdlib.h>

time_t mytime(void);
// (re)-start clock
#define TIC(i) (time_for_tic[i] = mytime())
// read clock time since last restart
#define TOC(i) (mytime() - time_for_tic[i])

#define TIC_TOC_COUNT (10)

time_t time_for_tic[TIC_TOC_COUNT];

/**
 * Clamp a value between a lower and a higher value
 * @param lower Lower bound
 * @param value value to clamp
 * @param upper  higher bound
 * @return Clamped value
 */
int clamp(int lower, int value, int upper);

/**
 * Swap two pointers
 * @param a First pointer to swap
 * @param b Second pointer to swap
 * @param t type of the pointers to swap
 */
#define swap_ptr(a, b, t) do { t tmp = *a; *a = *b; *b = tmp; } while(0);

#endif //HG_C_BENCHMARKS_UTIL_H

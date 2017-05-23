//
// Created by baldr on 5/2/17.
//
#include "util.h"

/**
 * gets current time in microseconds
 *
 * @return time in microseconds
 */
time_t mytime(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_usec + now.tv_sec * 1000000);
}

int clamp(int lower, int val, int higher) {
    return val < lower ? lower : higher < val ? higher : val;
}

void swap_ptr(void **ptr, void **other) {
    void *tmp = *ptr;
    *ptr = *other;
    *other = tmp;
}

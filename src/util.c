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

size_t clamp(size_t lower, size_t val, size_t higher) {
    return val < lower ? lower : higher < val ? higher : val;
}

void swap(void * ptr, void *other) {
    void *tmp = ptr;
    ptr = other;
    other = tmp;
}

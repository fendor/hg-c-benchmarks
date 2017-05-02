//
// Created by baldr on 5/2/17.
//
#include "util.h"
#include <stdlib.h>
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


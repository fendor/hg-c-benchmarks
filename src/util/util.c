//
// Created by baldr on 5/2/17.
//
#include <stdio.h>
#include "util.h"

time_t mytime(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_usec + now.tv_sec * 1000000);
}

int clamp(int lower, int val, int higher) {
    return val < lower ? lower : higher < val ? higher : val;
}


void bail_out(char *string) {
    fprintf(stderr, "Error: %s: %s", pgmname, string);
    exit(2);
}
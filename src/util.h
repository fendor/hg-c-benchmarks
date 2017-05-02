//
// Created by baldr on 5/2/17.
//
#ifndef HG_C_BENCHMARKS_UTIL_H
#define HG_C_BENCHMARKS_UTIL_H

#include <sys/time.h>

time_t mytime(void);
// (re)-start clock
#define TIC(i) (time_for_tic[i] = mytime())
// read clock time since last restart
#define TOC(i) (mytime() - time_for_tic[i])

#define TIC_TOC_COUNT (10)

time_t time_for_tic[TIC_TOC_COUNT];


#endif //HG_C_BENCHMARKS_UTIL_H

//
// Created by baldr on 7/13/17.
//

#ifndef HG_C_BENCHMARKS_NBODY_UTIL_H
#define HG_C_BENCHMARKS_NBODY_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <stdbool.h>
#include "../util/util.h"

// TODO: change signature: prefer arrays
/**
 * Represents a body, flowing in a 3D system
 */
struct Float3D {
    double x;
    double y;
    double z;
};

/**
 * Struct of optional arguments.
 * All arguments must always be set in order to avoid problems.
 * The benchmarks do not make sense without all variables set.
 */
struct arguments {
    int size;
    int iterations;
    int number_of_processes;
    bool debug;
};

/**
 * Typedef for easier access
 */
typedef struct Float3D Float3D;

/**
 * Typedef for easier access
 */
typedef struct arguments Args;

/**
 * Helper function to fil a planet according to the haskell implementation
 *
 * @param p planet to fill with coordinates
 * @param i offset of the planet, can be a index
 */
void fill_planet(Float3D *p, int i);

/**
 * Free the allocated memory
 *
 * @param planets Pointer to planets
 * @param buffer Pointer to buffer
 * @param p_file fd to some file you want to have closed
 * @param q_file fd to some file you want to have closed
 */
void free_resources(Float3D *planets, Float3D *buffer, FILE *p_file, FILE *q_file);

/**
 * Helper Function to write the planet array to an file descriptor
 *
 * @param fd File descriptor to write to
 * @param planets Planets that shall be written
 * @param size Number of Planets
 */
void pretty_print(FILE *fd, Float3D *planets, int size);

/**
 * Parse the arguments of this program
 * @param argc number of arguments
 * @param argv argument vector
 */
Args *parse_args(int argc, char **argv);

/**
 * Helper function, prints arguments of this program in a readable manner
 */
void print_args(Args *args);

/**
 * Append a line of the benchmark to an fd.
 * fd mustn't be null
 * @param fd File Descriptor of the csv file to append the output to
 * @param seq_t Time of this benchmark
 */
void append_nbody_csv(FILE *fd, Args *args, time_t seq_t);

#endif //HG_C_BENCHMARKS_NBODY_UTIL_H

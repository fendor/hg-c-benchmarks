#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <stdbool.h>
#include "util.h"

#define G (9.8)
#define EPS (0.005)

struct Float3D {
    double x;
    double y;
    double z;
};

struct {
    size_t size;
    size_t iterations;
    size_t numberOfProcesses;
    bool debug;
} args;

typedef struct Float3D Float3D;

static char *pgmname;

/**
 * Helper function to fil a planet according to the haskell implementation
 *
 * @param p planet to fill with coordinates
 * @param i offset of the planet, can be a index
 */
static void fill_planet(Float3D *p, int i) {
    p->x = i * 1.0;
    p->y = i * 0.2;
    p->z = i * 30.0;
}

/**
 * Free the allocated memory
 *
 * @param planets Pointer to planets
 * @param buffer Pointer to buffer
 */
static void free_resources(Float3D *planets, Float3D *buffer);

/**
 * Helper Function to write the planet array to an file descriptor
 *
 * @param fd File descriptor to write to
 * @param planets Planets that shall be written
 * @param size Number of Planets
 */
static void pretty_print(FILE *fd, Float3D *planets, size_t size);

/**
 * Shows usage of this program
 */
static void usage();

/**
 * Parse the arguments of this program
 * @param argc number of arguments
 * @param argv argument vector
 */
static void parse_args(int argc, char **argv);

/**
 * Helper function, prints arguments of this program in a readable manner
 */
static void print_args(void);

/**
 * Calculates the pair wise acceleration of two planets, represented as a triple.
 * Output is written to the outpur vectors
 *
 * @param p1 acceleration of this planet
 * @param p2 acceleration caused by this planet
 * @param out stored acceleration
 */
static void pair_wise_accel(Float3D p1, Float3D p2, Float3D *out);

/**
 * Compute acceleration of one planet to all its neighbours.
 * Stores the result in the output buffer
 *
 * @param planets Planets that are being simulated
 * @param buffer Buffer to save results to
 * @param index Index of the planet which accerlations is being computed
 * @param size Size of the overall planets
 */
static void accel(Float3D *planets, Float3D *buffer, int index, size_t size) {
    Float3D acc = {.x = 0.0, .y = 0.0, .z = 0.0};
    for (int i = 0; i < size; i++) {
        pair_wise_accel(planets[index], planets[i], &acc);
    }
    buffer[index].x = G * acc.x;
    buffer[index].y = G * acc.y;
    buffer[index].z = G * acc.z;
}

/**
 * Runs the simulation for a given number of planets for a given number of times
 * Results are stored in an output buffer
 *
 * @param planets planets that are being simulated
 * @param buffer buffer to store the results
 * @param numberOfPlanets number of planets that are being simulated
 * @param iterations number of iterations the simulation should be run
 */
static void run(Float3D *planets, Float3D *buffer, size_t numberOfPlanets, size_t iterations) {
    for (int i = 0; i < iterations; i++) {
#pragma omp parallel for num_threads(args.numberOfProcesses)
        for (int val = 0; val < numberOfPlanets; val++) {
            accel(planets, buffer, val, numberOfPlanets);
        }

        swap_ptr((void **) &planets, (void **) &buffer);
    }
}

int main(int argc, char **argv) {
    pgmname = argv[0];
    parse_args(argc, argv);
    if (args.debug) {
        print_args();
    }

    Float3D *planets = (Float3D *) malloc(sizeof(Float3D) * args.size);
    Float3D *buffer = (Float3D *) malloc(sizeof(Float3D) * args.size);

    if (planets != NULL && buffer != NULL) {

        for (int i = 0; i < args.size; i++) {
            fill_planet(&planets[i], i);
        }
        printf("Starting Kernel...\n");
        TIC(0);
        run(planets, buffer, args.size, args.iterations);
        time_t seq_t = TOC(0);
        printf("Kernel time: %zu.%06zus\n", seq_t / 1000000, seq_t % 1000000);

        FILE *fd = fopen("../nbody.res", "w+");
        if (fd != NULL) {
            pretty_print(fd, planets, args.size);
        } else {
            pretty_print(stderr, planets, args.size);
        }
    } else {
        printf("Could not allocate with malloc!");
    }
    free_resources(planets, buffer);
    return 0;
}

static void pretty_print(FILE *fd, Float3D *planets, size_t size) {
    for (int i = 0; i < size; i++) {
        Float3D p = planets[i];
        fprintf(fd, "(%g, %g, %g)\n", p.x, p.y, p.z);
    }
    fflush(fd);
}

static void parse_args(int argc, char **argv) {
    // default values according to the haskell program
    args.size = 3;
    args.iterations = 10;
    args.numberOfProcesses = 1;
    args.debug = false;

    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?dn:p:s:")) != -1) {
        switch (c) {
            case 'd':
                args.debug = true;
                break;
            case 'n':
                args.iterations = (size_t) strtol(optarg, NULL, 10);
                break;
            case 'p':
                args.numberOfProcesses = (size_t) strtol(optarg, NULL, 10);
                break;
            case 's':
                args.size = (size_t) strtol(optarg, NULL, 10);
                break;
            case '?':
                usage();
                break;
            default:
                usage();
                break;
        }
    }

    if (args.size <= 0 || args.numberOfProcesses <= 0) {
        usage();
    }
}

static void print_args(void) {
    printf("Args -> number of Planets: %zu, iterations: %zu, processes: %zu\n", args.size,
           args.iterations, args.numberOfProcesses
    );
}

void usage() {
    fprintf(stderr, "SYNOPSIS: %s [-d] [-p number_of_processes] [-s number_of_planets] [-n iterations]\n", pgmname);
    exit(1);
}

static void pair_wise_accel(Float3D p1, Float3D p2, Float3D *out) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    double distance_sq = dx * dx + dy * dy + dz * dz + EPS;
    double factor = 1.0 / sqrt(distance_sq * distance_sq * distance_sq);
    out->x += dx * factor;
    out->y += dy * factor;
    out->z += dz * factor;
}

static void free_resources(Float3D *planets, Float3D *buffer) {
    if (planets != NULL) {
        free(planets);
    }
    if (buffer != NULL) {
        free(buffer);
    }
}

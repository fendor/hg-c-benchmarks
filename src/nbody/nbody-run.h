//
// Created by baldr on 7/13/17.
//

#ifndef HG_C_BENCHMARKS_NBODY_RUN_H
#define HG_C_BENCHMARKS_NBODY_RUN_H


#include "../util/util.h"
#include "nbody-util.h"

#define G (9.8)
#define EPS (0.005)

/**
 * Calculates the pair wise acceleration of two planets, represented as a triple.
 * Output is written to the outpur vectors
 *
 * @param p1 acceleration of this planet
 * @param p2 acceleration caused by this planet
 * @param out stored acceleration
 */
void pair_wise_accel(Float3D p1, Float3D p2, Float3D *out);

/**
 * Compute acceleration of one planet to all its neighbours.
 * Stores the result in the output buffer
 *
 * @param planets Planets that are being simulated
 * @param buffer Buffer to save results to
 * @param index Index of the planet which accerlations is being computed
 * @param size Size of the overall planets
 */
void accel(Float3D *planets, Float3D *buffer, int index, ssize_t size);

/**
 * Runs the simulation for a given number of planets for a given number of times
 * Results are stored in an output buffer
 *
 * @param planets planets that are being simulated
 * @param buffer buffer to store the results
 * @param number_of_planets number of planets that are being simulated
 * @param iterations number of iterations the simulation should be run
 * @param number_of_processes that shall be used to run
 */
void run(Float3D *planets, Float3D *buffer, ssize_t number_of_planets, ssize_t iterations, ssize_t number_of_processes);

#endif //HG_C_BENCHMARKS_NBODY_RUN_H

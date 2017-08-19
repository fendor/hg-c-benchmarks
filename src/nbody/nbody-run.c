//
// Created by baldr on 7/13/17.
//
#include "nbody-run.h"

void pair_wise_accel(Float3D *p1, Float3D *p2, int i, int j, double *x, double *y, double *z) {
    // TODO: vectorise
    double dx = p2->x[j] - p1->x[i];
    double dy = p2->y[j] - p1->y[i];
    double dz = p2->z[j] - p1->z[i];
    double distance_sq = dx * dx + dy * dy + dz * dz + EPS;
    double factor = 1.0 / sqrt(distance_sq * distance_sq * distance_sq);
    *x += dx * factor;
    *y += dy * factor;
    *z += dz * factor;
}

void accel(Float3D *planets, Float3D *buffer, int index, ssize_t size) {
    double x = 0.0, y = 0.0, z = 0.0;
#pragma omp simd
    for (int i = 0; i < size; i++) {
        pair_wise_accel(planets, planets, index, i, &x, &y, &z);
    }
    buffer->x[index] = G * x;
    buffer->y[index] = G * y;
    buffer->z[index] = G * z;
}

void
run(Float3D *planets, Float3D *buffer, ssize_t number_of_planets, ssize_t iterations, ssize_t number_of_processes) {
    for (int i = 0; i < iterations; i++) {
#pragma omp parallel for num_threads(number_of_processes)
        for (int val = 0; val < number_of_planets; val++) {
            accel(planets, buffer, val, number_of_planets);
        }

        swap_ptr(&planets, &buffer, Float3D *);
    }
}
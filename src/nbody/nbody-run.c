//
// Created by baldr on 7/13/17.
//
#include "nbody-run.h"

void pair_wise_accel(Float3D p1, Float3D p2, Float3D *out) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    double distance_sq = dx * dx + dy * dy + dz * dz + EPS;
    double factor = 1.0 / sqrt(distance_sq * distance_sq * distance_sq);
    out->x += dx * factor;
    out->y += dy * factor;
    out->z += dz * factor;
}

void accel(Float3D *planets, Float3D *buffer, int index, int size) {
    Float3D acc = {.x = 0.0, .y = 0.0, .z = 0.0};
    for (int i = 0; i < size; i++) {
        pair_wise_accel(planets[index], planets[i], &acc);
    }
    buffer[index].x = G * acc.x;
    buffer[index].y = G * acc.y;
    buffer[index].z = G * acc.z;
}

void
run(Float3D *planets, Float3D *buffer, int number_of_planets, int iterations, int number_of_processes) {
    for (int i = 0; i < iterations; i++) {
#pragma omp parallel for num_threads(number_of_processes)
        for (int val = 0; val < number_of_planets; val++) {
            accel(planets, buffer, val, number_of_planets);
        }

        swap_ptr(&planets, &buffer, Float3D *);
    }
}
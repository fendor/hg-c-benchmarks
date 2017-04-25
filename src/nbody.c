#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define G (9.8f)
#define EPS (0.005f)

struct Float3D {
    double x;
    double y;
    double z;
};

typedef struct Float3D Float3D;

static Float3D *planets = NULL;
static Float3D *copy_place = NULL;

static unsigned int size = 3;
static unsigned int iterations = 10;

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

// (re)-start clock
#define TIC(i) (time_for_tic[i] = mytime())
// read clock time since last restart
#define TOC(i) (mytime() - time_for_tic[i])

#define TIC_TOC_COUNT 10

time_t time_for_tic[TIC_TOC_COUNT];

static void fill_planet(Float3D *p, int i) {
    p->x = i * 1.0f;
    p->y = i * 0.2f;
    p->z = i * 30.0f;
}

static void free_resources(void) {
    if (planets != NULL) {
        free(planets);
    }
    if (copy_place != NULL) {
        free(copy_place);
    }
}

static void pretty_print(void);

static void pretty_planet_print(Float3D p, char *msg);

static void pair_wise_accel(Float3D p1, Float3D p2, Float3D *out) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    double distance_sq = dx * dx + dy * dy + dz * dz + EPS;
    double factor = 1.0f / sqrt(distance_sq * distance_sq * distance_sq);
    out->x += dx * factor;
    out->y += dy * factor;
    out->z += dz * factor;
}

static void accel(int index) {
    Float3D acc = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
    for (int i = 0; i < size; i++) {
        pair_wise_accel(planets[index], planets[i], &acc);
    }
    copy_place[index].x = G * acc.x;
    copy_place[index].y = G * acc.y;
    copy_place[index].z = G * acc.z;
}

static void swap(void) {
    Float3D *tmp = copy_place;
    copy_place = planets;
    planets = tmp;
}

static void run() {
    for (int i = 0; i < iterations; i++) {
        for (int val = 0; val < size; val++) {
            accel(val);
        }
        swap();
    }
}

int main(int argc, char **argv) {
    if (argc > 1) {
        size = (unsigned int) strtol(argv[1], NULL, 10);
    }
    if (argc > 2) {
        iterations = (unsigned int) strtol(argv[2], NULL, 10);
    }

    planets = (Float3D *) malloc(sizeof(Float3D) * size);
    copy_place = (Float3D *) malloc(sizeof(Float3D) * size);

    for (int i = 0; i < size; i++) {
        fill_planet(&planets[i], i);
    }
    printf("Starting Kernel...\n");
    TIC(0);
    run();
    time_t seq_t = TOC(0);
    printf("Kernel time: %zu.%zus\n", seq_t / 1000000, seq_t % 1000000);
    // pretty_print();
    free_resources();
    return 0;
}

static void pretty_planet_print(Float3D p, char *msg) {
    if (msg != NULL) {
        printf("%s: Planet-> x: %f\t y: %f\t z: %f\n", msg, p.x, p.y, p.z);
    } else {
        printf("Planet-> x: %f\t y: %f\t z: %f\n", p.x, p.y, p.z);
    }
}

static void pretty_print() {
    for (int i = 0; i < size; i++) {
        Float3D p = planets[i];
        pretty_planet_print(p, NULL);
    }
}


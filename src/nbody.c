#include <stdio.h>
#include <stdlib.h>
#include "nbody/nbody-run.h"

int main(int argc, char **argv) {
    pgmname = argv[0];
    Args *args = parse_args(argc, argv);
    if (args->debug) {
        print_args(args);
    }

    Float3D *planets = (Float3D *) malloc(sizeof(Float3D) * args->size);
    Float3D *buffer = (Float3D *) malloc(sizeof(Float3D) * args->size);

    FILE *res = fopen("../nbody.time.res", "a+");
    FILE *check = fopen("../nbody.res", "w+");

    if (planets != NULL && buffer != NULL && res != NULL && check != NULL) {
        for (int n = 0; n < 1; ++n) {
            for (int i = 0; i < args->size; i++) {
                fill_planet(&planets[i], i);
            }
            printf("Starting Kernel...\n");
            TIC(0);
            run(planets, buffer, args->size, args->iterations, args->number_of_processes);
            time_t seq_t = TOC(0);
            printf("Kernel time: %zi.%06zis\n", seq_t / 1000000, seq_t % 1000000);

            append_nbody_csv(res, args, seq_t);


        }
        pretty_print(check, planets, args->size);

    } else {
        free_resources(planets, buffer, res, check);
        bail_out("Resources could not be allocated");
    }
    free_resources(planets, buffer, res, check);
    return 0;
}

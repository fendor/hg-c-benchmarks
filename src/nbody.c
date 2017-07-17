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

    if (planets != NULL && buffer != NULL) {
        FILE *res = fopen("../nbody.time.res", "a+");
        FILE *check = fopen("../nbody.res", "w+");
        for (int n = 0; n < 10; ++n) {
            for (int i = 0; i < args->size; i++) {
                fill_planet(&planets[i], i);
            }
            printf("Starting Kernel...\n");
            TIC(0);
            run(planets, buffer, args->size, args->iterations, args->number_of_processes);
            time_t seq_t = TOC(0);
            printf("Kernel time: %zi.%06zis\n", seq_t / 1000000, seq_t % 1000000);

            if (res != NULL) {
                append_nbody_csv(res, args, seq_t);
            }

        }
        if (res != NULL) {
            fflush(res);
            fclose(res);
        }
        if (check != NULL) {
            pretty_print(check, planets, args->size);
            fflush(check);
            fclose(check);
        }
    } else {
        printf("Could not allocate with malloc!");
    }
    free_resources(planets, buffer);
    return 0;
}

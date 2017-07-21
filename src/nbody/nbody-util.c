//
// Created by baldr on 7/13/17.
//

#include "nbody-util.h"

void pretty_print(FILE *fd, Float3D *planets, ssize_t size) {
    for (int i = 0; i < size; i++) {
        Float3D p = planets[i];
        fprintf(fd, "(%g, %g, %g)\n", p.x, p.y, p.z);
    }
    fflush(fd);
}


/**
 * Prints Synopsis of the program
 */
void usage() {
    fprintf(stderr, "SYNOPSIS: %s [-d] [-p number_of_processes] [-s number_of_planets] [-n iterations]\n", pgmname);
    exit(1);
}

Args *parse_args(int argc, char **argv) {
    // default values according to the haskell program
    Args *args = (Args *) malloc(sizeof(Args));
    args->size = 3;
    args->iterations = 10;
    args->number_of_processes = 1;
    args->debug = false;

    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?dn:p:s:")) != -1) {
        switch (c) {
            case 'd':
                args->debug = true;
                break;
            case 'n':
                args->iterations = strtol(optarg, NULL, 10);
                break;
            case 'p':
                args->number_of_processes = strtol(optarg, NULL, 10);
                break;
            case 's':
                args->size = strtol(optarg, NULL, 10);
                break;
            case '?':
                free(args);
                usage();
                break;
            default:
                free(args);
                usage();
                break;
        }
    }

    /* sanity checks*/
    if (args->size <= 0 || args->number_of_processes <= 0 || args->iterations < 0) {
        free(args);
        usage();
    }
    return args;
}

void free_resources(Float3D *planets, Float3D *buffer, FILE *p_file, FILE *q_file) {
    if (planets != NULL) {
        free(planets);
    }
    if (buffer != NULL) {
        free(buffer);
    }

    if (p_file != NULL) {
        fflush(p_file);
        fclose(p_file);
    }
    if (q_file != NULL) {
        fflush(q_file);
        fclose(q_file);
    }
}

void print_args(Args *args) {
    printf("Args -> number of Planets: %zi, iterations: %zi, processes: %zi\n", args->size, args->iterations,
           args->number_of_processes
    );
}


void fill_planet(Float3D *p, int i) {
    p->x = i * 1.0;
    p->y = i * 0.2;
    p->z = i * 30.0;
}


void append_nbody_csv(FILE *fd, Args *args, time_t seq_t) {
    fprintf(fd, "%zi,%zi,%zi,%zi\n", args->number_of_processes, args->size, args->iterations, seq_t);
}

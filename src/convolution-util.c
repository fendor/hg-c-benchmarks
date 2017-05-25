//
// Created by baldr on 5/23/17.
//

#include "convolution-util.h"


#define MAX_SIZE 16384

Image *read_image_from_fd(FILE *fd, Args *args) {
    if (fd != NULL) {
        char buf[MAX_SIZE];
        if (fgets(buf, MAX_SIZE, fd) != NULL) { ;
            char *token = NULL;
            token = strtok(buf, " ");
            if (NULL == token) bail_out("height couldn't be read from file");
            int height = (int) strtol(token, NULL, 10);
            token = strtok(NULL, " ");
            if (NULL == token) bail_out("width couldn't be read from file");
            int width = (int) strtol(token, NULL, 10);

            Image *img = init_image(width, height, 0.0);

            for (int y = 0; y < height; ++y) {
                if (NULL != fgets(buf, MAX_SIZE, fd)) {
                    token = strtok(buf, " ");
                    for (int x = 0; x < width; ++x) {
                        if (token != NULL) {
                            img->image[y][x] = strtod(token, NULL);
                        } else {
                            bail_out("the file dimensions were not correct, less number of lines than given width");
                        }
                        token = strtok(NULL, " ");
                    }
                } else {
                    bail_out("the file did not have enough lines to read");
                }
            }
            args->width = width;
            args->height = height;
            return img;
        }
    } else {
        bail_out("file descriptor couldn't was closed");
    }
    return NULL;
}


void print_image(Image *img) {
    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            char *format = NULL;
            if (img->image[y][x] < 0) {
                format = "%.05f\t";
            } else {
                format = " %.05f\t";
            }
            printf(format, img->image[y][x]);
        }
        printf("\n");
    }
}

void bail_out(char *string) {
    fprintf(stderr, "Error: %s: %s", pgmname, string);
    exit(2);
}

double sum_all(Image *img) {
    double val = 0.0;
    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            val += img->image[y][x];
        }
    }
    return val;
}

void write_checksum_to(FILE *fd, double checksum) {
    if (fd != NULL) {
        fprintf(fd, "%f\n", checksum);
    } else {
        fprintf(stdout, "%f\n", checksum);
    }
}


void free_image(Image *image) {
    if (image != NULL) {
        for (int height = 0; height < image->height; ++height) {
            if (image->image[height] != NULL) {
                free(image->image[height]);
            }
        }
        if (image->image != NULL) {
            free(image->image);
        }
        free(image);
    }
}

Image *init_image(int width, int height, double default_val) {
    Image *img = (Image *) malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->image = (double **) malloc(sizeof(double *) * height);
    for (int y = 0; y < img->height; ++y) {
        img->image[y] = (double *) malloc(sizeof(double) * width);
        for (int x = 0; x < img->width; ++x) {
            img->image[y][x] = default_val;
        }
    }
    return img;
}

Image *copy_shape(Image *img) {
    if (img != NULL) {
        return init_image(img->width, img->height, 0.0);
    } else {
        return NULL;
    }
}

int copy_image(Image *from, Image *to) {
    if (from != NULL && to != NULL) {
        if (from->width == to->width && from->height == to->height) {
            for (int y = 0; y < from->height; ++y) {
                for (int x = 0; x < from->width; ++x) {
                    to->image[y][x] = from->image[y][x];
                }
            }
            return 0;
        } else {
            return -2;
        }
    } else {
        return -1;
    }
}

Image *get_default_kernel(void) {
    Image *img = init_image(5, 5, 0);
    for (int height = 0; height < img->height; ++height) {
        for (int width = 0; width < img->width; ++width) {
            if ((height + width) % 2 == 0) {
                img->image[height][width] = 2;
            } else {
                img->image[height][width] = -2;
            }

            if (height == 2 && width == 2) {
                img->image[height][width] = -1;
            }
        }
    }
    return img;
}


void print_args(Args *args) {
    printf("Args: width: %d, height: %d, iterations: %d, processes: %d\n", args->width, args->height,
           args->number_of_iterations, args->number_of_processes
    );
    printf("\topt_image_file: %d\n", args->opt_image_from_file);
    if (args->opt_image_from_file) {
        printf("\topt_image_file_path: %s\n", args->image_file_path);
    }
    printf("\topt_kernel_file: %d\n", args->opt_kernel_from_file);
    if (args->opt_kernel_from_file) {
        printf("\topt_image_file_path: %s\n", args->kernel_file_path);
    }
    printf("\tdebug_mode: %d\n", args->debug);
}

Args *parse_args(int argc, char **argv) {
    Args *args = (Args *) malloc(sizeof(Args));

    // default values according to the haskell program
    args->number_of_iterations = 4000;
    args->width = 1024;
    args->height = 1024;
    args->number_of_processes = 1;
    args->debug = false;
    args->opt_image_from_file = false;
    args->opt_kernel_from_file = false;
    args->opt_width = false;
    args->opt_height = false;
    // parse the args
    int c;
    while ((c = getopt(argc, argv, "?dn:p:w:h:k:f:")) != -1) {
        switch (c) {
            case 'd':
                args->debug = true;
                break;
            case 'k':
                args->opt_kernel_from_file = true;
                args->kernel_file_path = optarg;
                break;
            case 'f':
                args->opt_image_from_file = true;
                args->image_file_path = optarg;
                break;
            case 'n':
                args->number_of_iterations = (int) strtol(optarg, NULL, 10);
                break;
            case 'p':
                args->number_of_processes = (int) strtol(optarg, NULL, 10);
                break;
            case 'w':
                args->opt_width = true;
                args->width = (int) strtol(optarg, NULL, 10);
                break;
            case 'h':
                args->opt_height = true;
                args->height = (int) strtol(optarg, NULL, 10);
                break;
            case '?':
                usage();
                break;
            default:
                usage();
                break;
        }
    }
    // sanity checks
    if (args->opt_image_from_file && (args->opt_width || args->opt_height)) {
        usage();
    }

    if (args->height <= 0 || args->width <= 0 || args->number_of_processes <= 0 || args->number_of_iterations < 0) {
        usage();
    }
    // sanity was verified
    return args;
}

void usage() {
    fprintf(stderr,
            "SYNOPSIS: %s [-d] [-p number_of_processes] ([-w width] [-h height] || -f image_file_name) [-k kernel_file_name] [-n iterations]\n",
            pgmname);
    exit(1);
}

double smart_access(Image *img, int x, int y) {
    return img->image[clamp(0, y, img->height - 1)][clamp(0, x, img->width - 1)];
}


void append_csv(FILE *fd, Image *img, Args *args, time_t time) {
    fprintf(fd, "%d,%d,%d,%d,%zu\n", args->number_of_processes, img->height, img->width, args->number_of_iterations,
            time);
}

void free_args(Args *args) {
    if (args != NULL) {
        free(args);
    }
}

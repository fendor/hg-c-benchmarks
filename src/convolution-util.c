//
// Created by baldr on 5/23/17.
//

#include "convolution-util.h"

#define MAX_SIZE 16384

Image *read_image_from_fd(FILE *fd, Args *args) {
    if (fd != NULL) {
        char buf[MAX_SIZE];
        if (fgets(buf, MAX_SIZE, fd) != NULL) { ;
            char *token = strtok(buf, " ");
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
            char format[100];
            if (img->image[y][x] < 0) {
                strcpy(format, "%.05f\t");
            } else {
                strcpy(format, " %.05f\t");
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

double get_checksum(Image *img) {
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


void append_csv(FILE *fd, ImageWithPadding *img, Args *args, time_t time) {
    fprintf(fd, "%d,%d,%d,%d,%zu\n", args->number_of_processes, img->inner_height, img->inner_width,
            args->number_of_iterations,
            time);
}

void free_args(Args *args) {
    if (args != NULL) {
        free(args);
    }
}

ImageWithPadding *add_padding(Image *img, int padding) {
    ImageWithPadding *padded_img = init_padded_image(img->width, img->height, padding);

    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            ACCESS_IMAGE(padded_img, x, y) = img->image[y][x];
        }
    }
    update_borders(padded_img);
    // unique hack
    for (int y = 0; y < padded_img->padding; ++y) {
        padded_img->image[y] = padded_img->image[padded_img->padding];
        padded_img->image[padded_img->inner_height + padded_img->padding + y] =
                padded_img->image[padded_img->inner_height + padded_img->padding - 1];
    }
    return padded_img;
}

Image *remove_padding(ImageWithPadding *padded_img) {
    Image *img = init_image(padded_img->inner_width, padded_img->inner_height, 0);

    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            img->image[y][x] = ACCESS_IMAGE(padded_img, x, y);
        }
    }
    return img;
}

void update_borders(ImageWithPadding *padded_img) {
    int width = padded_img->width;
    // TODO: all three loops can be merged into one: would this be reasonable?
    // left and right bounds
    for (int y = 0; y < padded_img->inner_height; ++y) {
        for (int x = 0; x < padded_img->padding; ++x) {
            int offset_y = y + padded_img->padding;
            ACCESS_FIELD(padded_img, x, offset_y) =
                    ACCESS_IMAGE(padded_img, 0, y);

            ACCESS_FIELD(padded_img, width - x - 1, offset_y) =
                    ACCESS_IMAGE(padded_img, padded_img->inner_width - 1, y);
        }
    }
    /*
    // upper and lower bounds
    for (int y = 0; y < padded_img->padding; ++y) {
        for (int x = 0; x < padded_img->inner_width; ++x) {
            int offset_x = x + padded_img->padding;
            ACCESS_FIELD(padded_img, offset_x, y) =
                    ACCESS_IMAGE(padded_img, x, 0);

            ACCESS_FIELD(padded_img, offset_x, height - y - 1) =
                    ACCESS_IMAGE(padded_img, x, padded_img->inner_height - 1);
        }
    }

    // edge cases
    double top_left_corner = ACCESS_IMAGE(padded_img, 0, 0);
    double bottom_left_corner = ACCESS_IMAGE(padded_img, 0, padded_img->inner_height - 1);
    double top_right_corner = ACCESS_IMAGE(padded_img, padded_img->inner_width - 1, 0);
    double bottom_right_corner = ACCESS_IMAGE(padded_img, padded_img->inner_width - 1, padded_img->inner_height - 1);

    for (int y = 0; y < padded_img->padding; ++y) {
        for (int x = 0; x < padded_img->padding; ++x) {
            int max_y = padded_img->inner_height + padded_img->padding + y;
            int max_x = padded_img->inner_width + padded_img->padding + x;
            ACCESS_FIELD(padded_img, x, y) = top_left_corner;
            ACCESS_FIELD(padded_img, x, max_y) = bottom_left_corner;
            ACCESS_FIELD(padded_img, max_x, y) = top_right_corner;
            ACCESS_FIELD(padded_img, max_x, max_y) = bottom_right_corner;
        }
    }
     */
}

void print_padded_image(ImageWithPadding *padded_img) {
    for (int y = 0; y < padded_img->height; ++y) {
        for (int x = 0; x < padded_img->width; ++x) {
            char format[100];
            if (padded_img->image[y][x] < 0) {
                strcpy(format, "%.05f\t");
            } else {
                strcpy(format, " %.05f\t");
            }
            printf(format, padded_img->image[y][x]);
        }
        printf("\n");
    }
}

void free_padded_image(ImageWithPadding *padded_img) {
    if (padded_img != NULL) {
        int height = padded_img->inner_height;
        for (int y = padded_img->padding; y < height; ++y) {
            if (padded_img->image[y] != NULL) {
                free(padded_img->image[y]);
            }
        }
        if (padded_img->image != NULL) {
            free(padded_img->image);
        }
        free(padded_img);
    }
}

int copy_padded_image(ImageWithPadding *padded_from, ImageWithPadding *padded_to) {
    if (padded_from == NULL || padded_to == NULL) {
        return -1;
    }
    if (padded_from->width != padded_to->width || padded_from->height != padded_to->width) {
        return -2;
    }
    int width = padded_from->width;
    int height = padded_from->height;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ACCESS_FIELD(padded_to, x, y) = ACCESS_FIELD(padded_from, x, y);
        }
    }

    return 0;
}

ImageWithPadding *init_padded_image(int inner_width, int inner_height, int padding) {
    ImageWithPadding *padded_img = (ImageWithPadding *) malloc(sizeof(ImageWithPadding));
    padded_img->padding = padding;
    padded_img->height = inner_height + 2 * padding;
    padded_img->width = inner_width + 2 * padding;
    padded_img->inner_height = inner_height;
    padded_img->inner_width = inner_width;

    int width = padded_img->width;
    int height = padded_img->height;

    padded_img->image = (double **) malloc(sizeof(double *) * height);
    for (int y = 0; y < height; ++y) {
        padded_img->image[y] = (double *) malloc(sizeof(double) * width);
        for (int x = 0; x < width; ++x) {
            padded_img->image[y][x] = 0;
        }
    }
    return padded_img;
}
/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, The i3lock-fancy-rapid authors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <png.h>
#include <sys/wait.h>

#include "screenshot.h"
#include "blur.h"
#include "operations.h"

#define power_of_2(x)   ((x) && (((x) & ((x)-1)) == 0))

static struct set_options {
    enum {
        BLUR = 1, PIXELATE = 2,
    } mode;

    int radius, times;

    enum {
        OVERLAY = 1, DIM = 2,
    } operations;

    const char *overlay_path;
    int overlay_x_offset, overlay_y_offset;
} set_options;

static void destroy_options(void) {
    free((void *) set_options.overlay_path);
}

static void usage(const char *program_name) {
    static const char msg[] = \
    "%s [-h] mode [operations]\n"
    "Modes:\n"
    "  --pixelate radius        Pixelates the screen. 'radius' defines the pixel size.\n"
    "  --blur radius:times      Blurs the screen more smoothly. 'times' defines how\n"
    "                           often blurring is applied.\n"
    "Operations:\n"
    "  --dim                    Dims the distorted screen.\n"
    "  --overlay png            Draws 'png' on top of the distorted screen.\n";
    fprintf(stderr, msg, program_name);
}

static const char *option_s = "hb:p:o:g:d";
static struct option options[] = {
        {"help",     no_argument,       0, 'h'},
        {"blur",     required_argument, 0, 'b'},
        {"pixelate", required_argument, 0, 'p'},
        {"overlay",  required_argument, 0, 'o'},
        {"offset",   required_argument, 0, 'g'},
        {"dim",      no_argument,       0, 'd'},
        {0, 0,                          0, 0},
};

static int setup_options(int argc, char *argv[]) {
    char flg;
    while ((flg = (char) getopt_long(argc, argv, option_s, options, 0)) != -1) {
        switch (flg) {
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'b':
                set_options.mode |= BLUR;
                if (sscanf(optarg, "%d:%d", &set_options.radius,
                           &set_options.times) != 2) {
                    return 1;
                }
                break;
            case 'p':
                set_options.mode |= PIXELATE;
                set_options.radius = atoi(optarg);
                break;
            case 'o':
                set_options.operations |= OVERLAY;
                set_options.overlay_path = strndup(optarg, PATH_MAX);
                break;
            case 'g':
                if (sscanf(optarg, "%d:%d", &set_options.overlay_x_offset,
                           &set_options.overlay_y_offset) != 2) {
                    /* TODO: log error */
                    return 1;
                }
                break;
            case 'd':
                set_options.operations |= DIM;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }
    return 0;
}

static void exit_on_error(const char *message) {
    fprintf(stderr, "%s\n", message);
    int status = 0;
    pid_t pid = fork();
    if (pid == 0) {
        execlp("notify-send", "notify-send", message, 0);
        exit(1);
    }
    wait(&status);
    if (status) {
        fprintf(stderr, "notify-send ended with status: %d\n", status);
    }
    exit(1);
}

int main(int argc, char *argv[]) {
    if (setup_options(argc, argv)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (set_options.radius < 0) {
        exit_on_error("Radius has to be non-negative!");
    }

    if (set_options.times < 0) {
        exit_on_error("Times has to be non-negative!");
    }

    if (!power_of_2(set_options.mode)) {
        exit_on_error("Exactly one mode has to be specified\n");
    }

    Screenshot screenshot = take_screenshot();

    unsigned char *buffer = malloc(screenshot.height * screenshot.width * 3);
    if (set_options.mode & BLUR) {
        box_blur(buffer, screenshot, set_options.radius, set_options.times);
    }

    if (set_options.mode & PIXELATE) {
        pixelate(buffer, screenshot, set_options.radius);
    }
    free(screenshot.data);
    screenshot.data = buffer;

    if (set_options.operations & DIM) {
        dim(&screenshot, 0.5);
    }

    if (set_options.operations & OVERLAY) {
        overlay(&screenshot, set_options.overlay_path,
                set_options.overlay_y_offset, set_options.overlay_x_offset);
    }

    write(STDOUT_FILENO, screenshot.data,
          screenshot.height * screenshot.width * 3);
    destroy_options();

    return EXIT_SUCCESS;
}
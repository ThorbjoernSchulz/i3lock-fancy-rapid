#include "blur.h"
#include <stdlib.h>
#include <string.h>

static void box_blur_h(unsigned char *dest, unsigned char *src,
                       int height, int width, int radius) {
    double coeff = 1.0 / (radius * 2 + 1);
#pragma omp parallel for
    for (int i = 0; i < height; ++i) {
        int iwidth = i * width;
        double r_acc = 0.0;
        double g_acc = 0.0;
        double b_acc = 0.0;
        for (int j = -radius; j < width; ++j) {
            if (j - radius - 1 >= 0) {
                int index = (iwidth + j - radius - 1) * 3;
                r_acc -= coeff * src[index];
                g_acc -= coeff * src[index + 1];
                b_acc -= coeff * src[index + 2];
            }
            if (j + radius < width) {
                int index = (iwidth + j + radius) * 3;
                r_acc += coeff * src[index];
                g_acc += coeff * src[index + 1];
                b_acc += coeff * src[index + 2];
            }
            if (j < 0) continue;
            int index = (iwidth + j) * 3;
            dest[index] = r_acc + 0.5;
            dest[index + 1] = g_acc + 0.5;
            dest[index + 2] = b_acc + 0.5;
        }
    }
}

static void box_blur_v(unsigned char *dest, unsigned char *src,
                       int height, int width,
                       int radius) {
    double coeff = 1.0 / (radius * 2 + 1);
#pragma omp parallel for
    for (int j = 0; j < width; ++j) {
        double r_acc = 0.0;
        double g_acc = 0.0;
        double b_acc = 0.0;
        for (int i = -radius; i < height; ++i) {
            if (i - radius - 1 >= 0) {
                int index = ((i - radius - 1) * width + j) * 3;
                r_acc -= coeff * src[index];
                g_acc -= coeff * src[index + 1];
                b_acc -= coeff * src[index + 2];
            }
            if (i + radius < height) {
                int index = ((i + radius) * width + j) * 3;
                r_acc += coeff * src[index];
                g_acc += coeff * src[index + 1];
                b_acc += coeff * src[index + 2];
            }
            if (i < 0) continue;
            int index = (i * width + j) * 3;
            dest[index] = r_acc + 0.5;
            dest[index + 1] = g_acc + 0.5;
            dest[index + 2] = b_acc + 0.5;
        }
    }
}

static void box_blur_once(unsigned char *dest, unsigned char *src,
                          int height, int width, int radius) {
    unsigned char *tmp = malloc(height * width * 3);
    box_blur_h(tmp, src, height, width, radius);
    box_blur_v(dest, tmp, height, width, radius);
    free(tmp);
}

void box_blur(unsigned char *dest, Screenshot s, int radius, int times) {
    box_blur_once(dest, s.data, s.height, s.width, radius);
    for (int i = 0; i < times - 1; ++i) {
        memcpy(s.data, dest, s.height * s.width * 3);
        box_blur_once(dest, s.data, s.height, s.width, radius);
    }
}

void pixelate(unsigned char *dest, Screenshot s, int radius) {
    radius = radius * 2 + 1;
#pragma omp parallel for
    for (int i = 0; i < s.height; i += radius) {
        for (int j = 0; j < s.width; j += radius) {
            int amount = 0;
            int r = 0;
            int g = 0;
            int b = 0;

            for (int k = 0; k < radius; ++k) {
                if (i + k >= s.height) break;

                for (int l = 0; l < radius; ++l) {
                    if (j + l >= s.width) break;

                    ++amount;
                    int index = ((i + k) * s.width + (j + l)) * 3;
                    r += s.data[index];
                    g += s.data[index + 1];
                    b += s.data[index + 2];
                }
            }

            r /= amount;
            g /= amount;
            b /= amount;

            for (int k = 0; k < radius; ++k) {
                if (i + k >= s.height) break;

                for (int l = 0; l < radius; ++l) {
                    if (j + l >= s.width) break;

                    int index = ((i + k) * s.width + (j + l)) * 3;
                    dest[index] = r;
                    dest[index + 1] = g;
                    dest[index + 2] = b;
                }
            }
        }
    }
}


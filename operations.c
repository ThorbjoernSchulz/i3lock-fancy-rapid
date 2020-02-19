#include <stdio.h>
#include <png.h>

#include "operations.h"

void overlay(Screenshot *s, const char *png_file,
             int off_y, int off_x) {
    FILE *file = fopen(png_file, "rb");
    if (!file) {
        perror(png_file);
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0,
                                                 0);
    if (!png_ptr) {
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        return;
    }

    png_init_io(png_ptr, file);
    png_read_png(png_ptr, info_ptr, 0, 0);

    png_uint_32 pwidth, pheight;
    int bit_depth, color_type, interlace_method, compression_method,
            filter_method;
    png_get_IHDR(png_ptr, info_ptr, &pwidth, &pheight, &bit_depth, &color_type,
                 &interlace_method, &compression_method, &filter_method);

    png_bytepp rows = png_get_rows(png_ptr, info_ptr);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    int pos = (off_y * s->width + off_x) * 3;
    for (unsigned int y = 0; y < pheight; ++y) {
        for (int x = 0; x < rowbytes; x += 4) {
            double alpha = (double) rows[y][x + 3] / 255.0;
            s->data[pos + 0] *= 1.0 - alpha;
            s->data[pos + 0] += rows[y][x + 0] * alpha;
            s->data[pos + 1] *= 1.0 - alpha;
            s->data[pos + 1] += rows[y][x + 1] * alpha;
            s->data[pos + 2] *= 1.0 - alpha;
            s->data[pos + 2] += rows[y][x + 2] * alpha;
            pos += 3;
        }
        pos += (s->width - rowbytes / 4) * 3;
    }
}


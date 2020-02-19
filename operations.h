#pragma once

#include "screenshot.h"

void dim(Screenshot *s, double factor);

void overlay(Screenshot *s, const char *png_file,
             int off_y, int off_x);

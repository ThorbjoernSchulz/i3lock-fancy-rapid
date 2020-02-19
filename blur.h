#pragma once

#include "screenshot.h"

void box_blur(unsigned char *dest, Screenshot s, int radius, int times);
void pixelate(unsigned char *dest, Screenshot s, int radius);

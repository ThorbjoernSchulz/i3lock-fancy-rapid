#pragma once

typedef struct Screenshot {
    int height, width;
    unsigned char *data;
} Screenshot;

Screenshot take_screenshot(void);

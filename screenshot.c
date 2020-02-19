#include "screenshot.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>

Screenshot take_screenshot(void) {
    Screenshot screenshot;

    Display *display = XOpenDisplay(NULL);
    Window root = XDefaultRootWindow(display);
    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);

    screenshot.height = gwa.height;
    screenshot.width = gwa.width;
    screenshot.data = malloc(screenshot.height * screenshot.width * 3);

    XImage *image =
            XGetImage(display, root, 0, 0, screenshot.width, screenshot.height,
                      AllPlanes, ZPixmap);
    for (int i = 0; i < screenshot.height; ++i) {
        int iwidth = i * screenshot.width;
        for (int j = 0; j < screenshot.width; ++j) {
            int index = (iwidth + j) * 3;
            unsigned long pixel = XGetPixel(image, j, i);
            screenshot.data[index] = (pixel & image->red_mask) >> 16;
            screenshot.data[index + 1] = (pixel & image->green_mask) >> 8;
            screenshot.data[index + 2] = pixel & image->blue_mask;
        }
    }
    XDestroyImage(image);
    XDestroyWindow(display, root);
    XCloseDisplay(display);

    return screenshot;
}

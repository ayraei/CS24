#ifndef PIXEL_H
#define PIXEL_H

#include "screen.h"

void draw_pixel(Screen *s, int x, int y,
                unsigned char value, unsigned char depth);

#endif /* PIXEL_H */


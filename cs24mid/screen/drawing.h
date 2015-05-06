#ifndef DRAWING_H
#define DRAWING_H

#include "screen.h"

void draw_hline(Screen *s, int x1, int x2, int y,
                unsigned char value, unsigned char depth);

void draw_vline(Screen *s, int x, int y, int y2,
                unsigned char value, unsigned char depth);

void draw_circle(Screen *s, int x, int y, int r,
                 unsigned char value, unsigned char depth);

#endif /* DRAWING_H */


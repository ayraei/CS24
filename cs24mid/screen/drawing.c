#include "drawing.h"
#include "pixel.h"
#include <stdlib.h>
#include <assert.h>


/* Draw a horizontal line. */
void draw_hline(Screen *s, int x1, int x2, int y,
                unsigned char value, unsigned char depth) {
    int x;

    for (x = x1; x < x2; x++)
        draw_pixel(s, x, y, value, depth);
}


/* Draw a vertical line. */
void draw_vline(Screen *s, int x, int y1, int y2,
                unsigned char value, unsigned char depth) {
    int y;

    for (y = y1; y < y2; y++)
        draw_pixel(s, x, y, value, depth);
}


/* This helper function draws 8 symmetric points that appear on a circle with
 * the specified center.
 */
void circle_points(Screen *s, int xc, int yc, int x, int y,
                   unsigned char value, unsigned char depth) {
    assert(s != NULL);

    draw_pixel(s, xc + x, yc + y, value, depth);
    draw_pixel(s, xc + x, yc - y, value, depth);
    draw_pixel(s, xc - x, yc + y, value, depth);
    draw_pixel(s, xc - x, yc - y, value, depth);

    draw_pixel(s, xc + y, yc + x, value, depth);
    draw_pixel(s, xc + y, yc - x, value, depth);
    draw_pixel(s, xc - y, yc + x, value, depth);
    draw_pixel(s, xc - y, yc - x, value, depth);
}


/* This function draws a circle with the specified location and radius using
 * the Bresenham circle drawing algorithm.
 */
void draw_circle(Screen *s, int xc, int yc, int r,
                 unsigned char value, unsigned char depth) {
    int x, y, d;

    assert(s != NULL);

    x = 0;
    y = r;
    d = 1 - r;

    circle_points(s, xc, yc, x, y, value, depth);

    while (y > x) {
        x++;
        if (d < 0) {
            d += 2 * x + 3;
        }
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        circle_points(s, xc, yc, x, y, value, depth);
    }
}



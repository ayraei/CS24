#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "screen.h"
#include "pixel.h"
#include "drawing.h"


/* This is a simple program to show off the screen drawing code.  It doesn't
 * look very pretty, mainly because a 39x25 screen with a funky aspect ratio
 * doesn't look pretty.  (Why 39?  Because when the screen is drawn, it will
 * fit within an 80-character-wide console.)
 */
int main() {
    Screen *s = make_screen(39, 25);
    int i;

    srand(time(NULL));

    /* This really isn't that exciting.
    printf("New screen:\n");
    print_screen(s);
    printf("\n");
    */

    printf("sizeof(Pixel) = %lu\n\n", sizeof(Pixel));

    printf("Drawing overlapping lines.  Outermost lines should be deepest, "
           "and should be\noccluded by nearer lines.  Part of each line is "
           "out of bounds.\n");
    for (i = 0; i < 24; i++) {
        if (i % 2 == 0)
            draw_hline(s, -3, 28, i, 1 + (i % MAX_COLOR), MAX_DEPTH - i);
        else
            draw_vline(s, i, -3, 28, 1 + (i % MAX_COLOR), MAX_DEPTH - i);
    }
    printf("Screen:\n");
    print_screen(s);
    printf("\n");

    clear_screen(s);
    printf("Drawing random points, some of which will be out of bounds.\n");
    for (i = 0; i < 30; i++) {
        int x = rand() % (s->width + 10);
        int y = rand() % (s->height + 10);

        int color = 1 + rand() % MAX_COLOR;
        int depth = rand() % MAX_DEPTH;

        draw_pixel(s, x, y, color, depth);
    }
    printf("Screen:\n");
    print_screen(s);
    printf("\n");

    clear_screen(s);
    printf("Drawing random circles.\n");
    for (i = 0; i < 6; i++) {
        int x = rand() % s->width;
        int y = rand() % s->height;
        int r = 3 + rand() % 5;

        int color = 1 + rand() % MAX_COLOR;
        int depth = rand() % MAX_DEPTH;

        draw_circle(s, x, y, r, color, depth);
    }
    printf("Screen:\n");
    print_screen(s);
    printf("\n");

    return 0;
}


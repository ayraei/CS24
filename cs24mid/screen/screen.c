#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define USE_COLOR 1


/* Allocates and initializes a new "screen" object of the specified size. */
Screen * make_screen(int width, int height) {
    assert(width > 0);
    assert(height > 0);

    Screen *s = malloc(sizeof(Screen) + width * height);

    s->width = width;
    s->height = height;
    clear_screen(s);

    return s;
}


/* Clears a screen by setting all pixels to ' ', the space character. */
void clear_screen(Screen *s) {
    int x, y;

    for (y = 0; y < s->height; y++) {
        for (x = 0; x < s->width; x++) {
            s->pixels[y * s->width + x].value = BLACK;
            s->pixels[y * s->width + x].depth = MAX_DEPTH;
        }
    }
}


/* Prints the contents of a screen to the console. */
void print_screen(Screen *s) {
    int x, y;

    printf("+");
    for (x = 0; x < s->width; x++)
        printf("--");
    printf("+\n");

    for (y = 0; y < s->height; y++) {
        printf("|");
        for (x = 0; x < s->width; x++) {
            unsigned char value = s->pixels[y * s->width + x].value;
#if USE_COLOR
            printf("\x1B[%d;30m  ", 40 + value % (MAX_COLOR + 1));
#else
            printf("%c%c", value > 0 ? 64 + value : 32,
                           value > 0 ? 64 + value : 32);
#endif
        }
#if USE_COLOR
        printf("\033[0m");
#endif
        printf("|\n");
    }

    printf("+");
    for (x = 0; x < s->width; x++)
        printf("--");
    printf("+\n");
}


/* Deallocates the memory used by a Screen object. */
void free_screen(Screen *s) {
    free(s);
}



#ifndef SCREEN_H
#define SCREEN_H


typedef struct Pixel {
    unsigned char value;
    unsigned char depth;
} Pixel;


/* Colors we can draw with */
#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

#define MAX_COLOR 7


/* The maximum depth - this is the value that is "farthest away". */
#define MAX_DEPTH 0xFF


/* A simple data structure to represent a screen with width * height
 * pixels.  Values written to the pixels are characters.
 */
typedef struct Screen {
    int width;
    int height;

    Pixel pixels[];
} Screen;


Screen * make_screen(int width, int height);
void clear_screen(Screen *s);
void print_screen(Screen *s);
void free_screen(Screen *s);


#endif /* SCREEN_H */


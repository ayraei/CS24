#include <stdio.h>


int gcd(int x, int y);


int main(int argc, char **argv) {
    int res, x, y;

    if (argc == 1) {
        printf("usage:  %s x y\n\n", argv[0]);
        printf("\tFor nonnegative arguments x, y, the program prints "
            "gcd(x, y).\n\n");
        return 1;
    }

    res = sscanf(argv[1], "%d", &x);
    if (res != 1) {
        printf("Unparseable input \"%s\".\n\n", argv[1]);
        return 1;
    }
    res = sscanf(argv[2], "%d", &y);
    if (res != 1) {
        printf("Unparseable input \"%s\".\n\n", argv[2]);
        return 1;
    }
    if (x <= 0 || y <= 0) {
        printf("One or both of the arguments is not positive.\n\n");
        return 1;
    }
    if (x < y)
        printf("gcd(%d, %d) = %d\n\n", x, y, gcd(y, x));
    else
        printf("gcd(%d, %d) = %d\n\n", x, y, gcd(x, y));

}

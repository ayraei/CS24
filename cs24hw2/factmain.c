#include <stdio.h>


int fact(int x);


int main(int argc, char **argv) {
    int res, n;

    if (argc == 1) {
        printf("usage:  %s n\n\n", argv[0]);
        printf("\tFor nonnegative argument 'n', the program prints n!.\n\n");
        return 1;
    }

    res = sscanf(argv[1], "%d", &n);
    if (res != 1) {
        printf("Unparseable input \"%s\".\n\n", argv[1]);
        return 1;
    }
    if (n < 0) {
        printf("%d is less than 0.\n\n", n);
        return 1;
    }

    printf("%d! = %d\n\n", n, fact(n));

}

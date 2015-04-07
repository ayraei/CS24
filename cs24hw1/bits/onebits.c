#include <stdio.h>


int count_onebits(unsigned int n);


int main(int argc, char **argv) {
    int i, res;
    unsigned int n;

    if (argc == 1) {
        printf("usage:  %s N1 [N2 ...]\n\n", argv[0]);
        printf("\tN1, N2, ... are positive integers\n\n");
        printf("\tFor each argument, the program prints out the number of\n"
               "\tbits that are 1 in the number.\n\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        res = sscanf(argv[i], "%u", &n);
        if (res == 1)
            printf("Input:  %u\tOne-bits:  %u\n\n", n, count_onebits(n));
        else
            printf("Unparseable input \"%s\".  Skipping.\n\n", argv[i]);
    }
}

/*
 * Given an unsigned integer n, this function returns the number of bits in n
 * that are 1.
 */
int count_onebits(unsigned int n) {
    /* Counter for number of one-bits. */
    int result = 0;
    
    /* Loop through each bit in n from right to left, counting one-bits. */
    while (n != 0) {
        if (n & 1 == 1) {
            result++;
        }
        n = n >> 1;
    }

    return result;
}


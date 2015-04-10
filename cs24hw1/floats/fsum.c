#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ffunc.h"


/* This function takes an array of single-precision floating point values,
 * and computes a sum in the order of the inputs.  Very simple.
 */
float fsum(FloatArray *floats) {
    float sum = 0;
    int i;

    for (i = 0; i < floats->count; i++)
        sum += floats->values[i];

    return sum;
}


/*
 * This function takes an array of single-precision floating point values,
 * and computes their sum more accurately than plain addition by introducing
 * a compensation term to try to hold onto as much precision as possible.
 *
 * From HW1 writeup:
 * Algebraically, what should 'c' always be equal to?
 * - It should be zero.
 *
 * In what situations would it deviate from this expected value?
 * - When lower-precision bits are being discarded, 'c' will not be zero.
 *
 * How can we take advantage of this to improve accuracy?
 * - We can try to add the difference resulting from discarding bits back into
 * the sum in the next iteration of the loop in order to maintain precision.
 *
 */
float my_fsum(FloatArray *floats) {
    float sum = 0.0, c = 0.0;   /* c is the compensation term */
    float nextsum, val;
    int i;
    
    for (i = 0; i < floats->count; i++) {
        /* Try to add compensation term back in. */
        val = floats->values[i] - c;
        nextsum = sum + val;
        /* 'c' = the negative difference of lost precision */
        c = (nextsum - sum) - val;
        sum = nextsum;
    }

    return sum;
}


int main() {
    FloatArray floats;
    float sum1, sum2, sum3, my_sum;

    load_floats(stdin, &floats);
    printf("Loaded %d floats from stdin.\n", floats.count);

    /* Compute a sum, in the order of input. */
    sum1 = fsum(&floats);

    /* Use my_fsum() to compute a sum of the values.  Ideally, your
     * summation function won't be affected by the order of the input floats.
     */
    my_sum = my_fsum(&floats);

    /* Compute a sum, in order of increasing magnitude. */
    sort_incmag(&floats);
    sum2 = fsum(&floats);

    /* Compute a sum, in order of decreasing magnitude. */
    sort_decmag(&floats);
    sum3 = fsum(&floats);

    /* %e prints the floating-point value in full precision,
     * using scientific notation.
     */
    printf("Sum computed in order of input:  %e\n", sum1);
    printf("Sum computed in order of increasing magnitude:  %e\n", sum2);
    printf("Sum computed in order of decreasing magnitude:  %e\n", sum3);

    printf("My sum:  %e\n", my_sum);

    return 0;
}


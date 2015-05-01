/*
#include "my_setjmp.h"
*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>


static jmp_buf env;


int main() {
    if (test1() == -1)
        fprintf(stdout, "long_jmp can cross multiple functions: PASS\n");
    if (test2() == 0)
        fprintf(stdout, "longjmp(buf, 0) returns 1: PASS\n");
    return 0;
}


int test1() {
    int x = 0;
    if (setjmp(env) == 0)
        return g(x);
    else
        return -1;
}

/* helper function for test1 */
int g(int x) {
    if (x == 0)
        longjmp(env, 1);
    return 2.0 / x;
}


int test2() {
    if (setjmp(env) == 1)
        return 0;
    else
        longjmp(env, 0);
}

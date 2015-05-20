#include <stdio.h>
#include "sthread.h"


/*
 * Thread 1 prints "Hello".
 * Thread 2 prints "Goodbye".
 */

/*! This thread-function prints "Hello" as many times as specified by 'arg'. */
static void loop1(void *arg) {
    int i;
    for (i = 0; i < (int)arg; i++) {
        printf("%d: Hello\n", i);
        sthread_yield();
    }
}

/*! This thread-function prints "Goodbye" 'arg' times. */
static void loop2(void *arg) {
    int i;
    for (i = 0; i < (int)arg; i++) {
        printf("%d: Goodbye\n", i);
        sthread_yield();
    }
}

/*
 * The main function starts the two loops in separate threads,
 * the start the thread scheduler.
 */
int main(int argc, char **argv) {
    sthread_create(loop1, (void *)3);
    sthread_create(loop2, (void *)10);
    sthread_start();
    return 0;
}



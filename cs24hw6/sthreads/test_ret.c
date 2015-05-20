#include <stdio.h>
#include "sthread.h"


/*! This thread-function prints "Hello" as many times as specified by 'arg'. */
static void loop(void *arg) {
    int i;
    int iarg = (int)arg;
    for (i = 0; i < iarg; i++) {
        printf("Thread %d, iteration %d: Hello\n", iarg, i);
        sthread_yield();
    }
}

/*
 * The main function starts four loops of different lengths in separate threads,
 * then starts the thread scheduler.
 */
int main(int argc, char **argv) {
    sthread_create(loop, (void *)1);      /* "Thread 1" */
    sthread_create(loop, (void *)4);      /* "Thread 2" */
    sthread_create(loop, (void *)2);      /* "Thread 4" */
    sthread_create(loop, (void *)3);      /* "Thread 3" */
    sthread_start();
    return 0;
}

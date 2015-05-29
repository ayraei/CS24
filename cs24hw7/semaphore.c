/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "semaphore.h"

/*
 * The semaphore data structure contains the single value of the semaphore.
 */
struct _semaphore {
     int i;
};


typedef struct _queue {
    Thread *head;
    Thread *tail;
} Queue;


static Queue blocked_queue;



/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {
    Semaphore *semp;

     semp = (Semaphore *)malloc(sizeof(Semaphore));
     semp->i = init;

    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {
    /* Reading the semaphore's value must be atomic. */
    __sthread_lock();
    int i = semp->i;
    __sthread_unlock();
    
    if (i == 0) {
        sthread_block();
        queue_append(&blocked_queue, sthread_current());
    }
    
    /* Altering the semaphore's value must be atomic. */
    __sthread_lock();
    semp->i--;
    __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {
    /* Altering the semaphore's value must be atomic. */
    __sthread_lock();
    semp->i++;
    __sthread_unlock();
    
    /* If there are blocked threads, unblock one. */
    if (blocked_queue.head != NULL) {
        Thread *t = queue_take(&blocked_queue);
        sthread_unblock(t);
        queue_remove(&blocked_queue, t);
    }
}


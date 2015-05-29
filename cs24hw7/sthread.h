/*
 * Simple thread API.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */
#ifndef _STHREAD_H
#define _STHREAD_H

#include "glue.h"

/*
 * Thread handles.
 */
typedef struct _thread Thread;

/*
 * Thread operations.
 */
Thread *sthread_create(ThreadFunction f, void *arg);
void sthread_yield(void);
void sthread_block(void);
Thread *sthread_current(void);
void sthread_unblock(Thread *threadp);


/*
 * Queue
 */
typedef struct _queue Queue;
void queue_append(Queue *queuep, Thread *threadp);
Thread *queue_take(Queue *queuep);
void queue_remove(Queue *queuep, Thread *threadp);


/*
 * The function called when a thread returns (which they shouldn't
 * do).
 */
void __sthread_return_handler(void);

/*
 * The start function should be called *once* in
 * the main() function of your program.  This function
 * never returns.
 */
void sthread_start(int timer);

#endif /* _STHREAD_H */


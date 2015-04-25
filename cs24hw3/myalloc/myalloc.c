/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
 int MEMORY_SIZE;
unsigned char *mem;


/* TODO:  The unacceptable allocator uses an external "free-pointer" to track
 *        where free memory starts.  If your allocator doesn't use this
 *        variable, get rid of it.
 *
 *        You can declare data types, constants, and statically declared
 *        variables for managing your memory pool in this section too.
 */
static unsigned char *freeptr;
#define HEADER_SIZE 8
#define INT_SIZE 4

/*
 * HEADER STRUCTURE, 8 bytes
 * First four bytes (int) is the [usable] size of the previous block.
 * Second four bytes (int) is the [usable] size of the current block.
 * These values are negative if the block is allocated, and positive if
 * the block is free. (Or zero, for boundary conditions.)
 */


/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {

    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n");
        abort();
    }

    /* Initialize entire memory pool as one free block followed by one empty
     * block at the end (boundary condition). */
    *((int *) mem) = 0;
    *((int *) (mem + INT_SIZE)) = MEMORY_SIZE - 2*HEADER_SIZE;
    
    *((int *) (mem + MEMORY_SIZE - HEADER_SIZE)) = MEMORY_SIZE - 2*HEADER_SIZE;
    *((int *) (mem + MEMORY_SIZE - HEADER_SIZE + INT_SIZE)) = 0;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size) {
    /* Pointer for the current block we are examining. */
    unsigned char *curr = mem;
    /* Amount of usable space in block curr. */
    int curr_space = *((int *) (curr + INT_SIZE));
    /* Amount of usable space in next block. */
    /* We use abs(curr_space) because it will be negative if allocated. */
    int next_space = *((int *) (curr + abs(curr_space) + INT_SIZE));
    
    /* Find the next available free block using first-fit strategy. */
    while (size > curr_space && next_space != 0) {
        /* Move to the next block. */
        curr += HEADER_SIZE + abs(curr_space);
        /* Recompute available space of new current and next blocks. */
        curr_space = *((int *) (curr + INT_SIZE));
        next_space = *((int *) (curr + abs(curr_space) + INT_SIZE));
    }
    
    /* Reached the end of memory block and found no usable free blocks. */
    if (size > curr_space) {
        fprintf(stderr, "myalloc: cannot service request of size %d\n", size);
        /*
        fprintf(stderr, "myalloc: cannot service request of size %d with"
                " %d bytes allocated\n", size, (freeptr - mem));
        */
        return (unsigned char *) 0;
    }
    
    /* Else, curr points to a usable free block, so we allocate it. */
    else {
        /* Free block can be split. */
        if (curr_space - size > 8) {
            /* Set the second header value of the current block. */
            *((int *) (curr + INT_SIZE)) = -size;
            /* Set the first header value of the next block. */
            *((int *) (curr + HEADER_SIZE + size)) = -size;
            /* Set the second header value of the next block. */
            *((int *) (curr + HEADER_SIZE + size + INT_SIZE)) =
                curr_space - size - HEADER_SIZE;
        }
        /* Else, free block cannot be split; allocate the whole thing. */
        else {
            /* Set the second header value of the current block. */
            *((int *) (curr + INT_SIZE)) *= -1;
            /* Set the first header value of the next block. */
            *((int *) (curr + HEADER_SIZE + curr_space)) = -curr_space;
        }
        
        return curr + HEADER_SIZE;
    }
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {
    oldptr -= HEADER_SIZE;
    int curr_space = *((int *) (oldptr + INT_SIZE)) * -1;
    *((int *) (oldptr + INT_SIZE)) *= -1;
    *((int *) (oldptr + HEADER_SIZE + curr_space)) *= -1;
}


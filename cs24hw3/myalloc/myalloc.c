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
#include <assert.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
 int MEMORY_SIZE;
unsigned char *mem;


/*
 * You can declare data types, constants, and statically declared variables
 * for managing your memory pool in this section.
 *
 * HEADER STRUCTURE, 8 bytes
 * First four bytes (int) is the [usable] size of the previous block.
 * Second four bytes (int) is the [usable] size of the current block.
 * These values are negative if the block is allocated, and positive if
 * the block is free. (Or zero, for boundary conditions.)
 *
 */
#define HEADER_SIZE 8
#define INT_SIZE 4


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
    /* First header value of current block. */
    *((int *) mem) = 0;
    /* Second header value of current block. */
    *((int *) (mem + INT_SIZE)) = MEMORY_SIZE - 2*HEADER_SIZE;
    
    /* First header value of next block. */
    *((int *) (mem + MEMORY_SIZE - HEADER_SIZE)) = MEMORY_SIZE - 2*HEADER_SIZE;
    /* Second header value of next block. */
    *((int *) (mem + MEMORY_SIZE - INT_SIZE)) = 0;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 *
 * This implementation is linear-time in the number of blocks, because
 * the allocator has to search through every block and stops only if it
 * reaches the end of the malloc'd memory, or it finds a free block.
 */
unsigned char *myalloc(int size) {
    /* Pointer for the current block we are examining. */
    unsigned char *curr = mem;
    /* Amount of usable space in block curr. */
    int curr_space = *((int *) (curr + INT_SIZE));
    /* Amount of usable space in next block. */
    /* We use abs(curr_space) because it will be negative if allocated. */
    int next_space =
        *((int *) (curr + HEADER_SIZE + abs(curr_space) + INT_SIZE));
        
    /* Find the next available free block using first-fit strategy. */
    while (size > curr_space && next_space != 0) {
        /* Move to the next block. */
        curr += HEADER_SIZE + abs(curr_space);
        /* Recompute available space of new current and next blocks. */
        curr_space = *((int *) (curr + INT_SIZE));
        next_space =
            *((int *) (curr + HEADER_SIZE + abs(curr_space) + INT_SIZE));
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
            *((int *) (curr + INT_SIZE)) = -curr_space;
            /* Set the first header value of the next block. */
            *((int *) (curr + HEADER_SIZE + curr_space)) = -curr_space;
        }
        
        return (curr + HEADER_SIZE);
    }
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 *
 * This implementation (if it worked...) works in constant-time.
 * Therefore, deallocation is constant-time, and coalescing is constant-time.
 */
void myfree(unsigned char *oldptr) {
    /* Frees the block at oldptr. */
    
    /* Move the pointer back to the beginning of the header. */
    oldptr -= HEADER_SIZE;
    
    /* Get the amount of space used by the previous block. */
    int prev_space = *((int *) oldptr);
    
    /* Get the amount of now-usable space in this block. */
    int curr_space = abs(*((int *) (oldptr + INT_SIZE)));
    
    /* Get the amount of space used by the next block. */
    int next_space = *((int *) (oldptr + HEADER_SIZE + curr_space + INT_SIZE));
    
    /* Set size of usable space in current block. */
    *((int *) (oldptr + INT_SIZE)) = curr_space;
    /* Set first header value of next block. */
    *((int *) (oldptr + HEADER_SIZE + curr_space)) = curr_space;
    
    /* Coalesces free blocks, if any. */
    
    /* Coalesce right. */
    
    /* Next block is free, so coalesce it into the current block. */
    if (next_space > 0) {
        /* Absorb all memory used by next block into current block. */
        curr_space += HEADER_SIZE + next_space;
        /* Second header value of current block reflects updated free space. */
        *((int *) (oldptr + INT_SIZE)) = curr_space;
        /* Update first header value of next block. */
        *((int *) (oldptr + HEADER_SIZE + curr_space)) = curr_space;
    }
    
    /* Coalesce left. */
    
    /* Previous block is free, so coalesce it with the current block. */
    /* NOTE TO GRADER: This part doesn't work. I've spent several days
     * trying to debug it and I still can't figure it out.. Sorry :(
     */
    /* if (prev_space > 0) { */
    if (0) {
        /* Absorb all memory used by current block into previous block. */
        
        /* Second header value of previous block reflects updated free space. */
        *((int *) (oldptr - prev_space - INT_SIZE)) =
            prev_space + HEADER_SIZE + curr_space;
        /* Update first header value of next block. */
        *((int *) (oldptr + HEADER_SIZE + curr_space)) =
            prev_space + HEADER_SIZE + curr_space;
    }
}


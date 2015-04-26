/* gcc -g -O0 myalloc.c simpletests.c -o simpletests */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myalloc.h"


#define POOL_SIZE 2000


/* Allocate a chunk of memory using myalloc(), and then write every byte in
 * the payload to a specific value.  This allows us to identify when the
 * allocator doesn't return the payload pointer properly, probably because
 * they didn't remember to adjust the pointer, or the block itself may not be
 * large enough.
 */
unsigned char * alloc_trample(int size, unsigned char value) {
    unsigned char *p;

    printf(" * Allocating %d bytes.\n", size);
    p = myalloc(size);

    if (p == NULL) {
        printf("   ERROR:  Couldn't allocate memory!\n");
        goto Done;
    }

    printf(" * Setting memory to 0x%02X.\n", value);
    memset(p, value, size);

Done:
    return p;
}


/* A very simplistic test to determine if two memory regions overlap. */
int overlaps(unsigned char *p1, int size1, unsigned char *p2, int size2) {
    int i;

    // See if p1 falls within p2's range.
    if (p1 >= p2 && p1 < p2 + size2)
        return 1;

    // See if p2 falls within p1's range.
    if (p2 >= p1 && p2 < p1 + size1)
        return 1;

    // They don't seem to overlap!
    return 0;
}


/* Single allocation.  Write entire payload.  Then release memory.
 *
 * If this test dies in myfree(), make sure you return your payload pointer
 * from myalloc() properly, and that your myfree() call correctly converts
 * the payload pointer back into a header pointer.
 *
 * The test writes the values 0xC5 to the payload; if you see these values
 * before or after the payload, you have a problem.
 */
void test_single_alloc() {
    unsigned char *p;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_single_alloc:\n");

    p = alloc_trample(100, 0xc5);

    printf(" * Releasing allocation.\n");
    myfree(p);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Single allocation, much larger than pool size.  Should return NULL!
 *
 * If this test fails, you have an error in how you determine that an
 * allocation request cannot be satisfied.
 */
void test_single_alloc_fail() {
    unsigned char *p;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_single_alloc_fail:\n");

    printf(" * Allocating 1000000 bytes.\n");
    p = myalloc(1000000);

    if (p != NULL) {
        printf("   ERROR:  Pool is only 2000 bytes, but got back a non-NULL "
               "pointer!\n");
    }

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Helper function to allocate two regions of memory, and do a few basic
 * operations on them.  The function returns 1 if successful, 0 if a failure
 * is encountered.
 */
int do_2_allocs(int size1, unsigned char **pp1,
                int size2, unsigned char **pp2) {
    unsigned char *p1 = NULL, *p2 = NULL;
    int rc = 1;

    p1 = alloc_trample(size1, 0xbb);
    if (p1 == NULL)
        goto Done;

    p2 = alloc_trample(size2, 0xdd);
    if (p2 == NULL)
        goto Done;

    if (overlaps(p1, size1, p2, size2)) {
        printf("   ERROR:  p1 and p2 overlap!\n");
        rc = 0;  // Failure!
    }

Done:
    if (!p1 || !p2)
        rc = 0;

    // Return the pointers to memory we allocated.
    *pp1 = p1;
    *pp2 = p2;

    return rc;
}


/* Two allocations p1 and p2.  Free p1 then p2. */
void test_2allocs_free12() {
    unsigned char *p1, *p2;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_2allocs_free12:\n");

    if (!do_2_allocs(400, &p1, 300, &p2))
        goto Done;

    printf(" * Freeing first allocation\n");
    myfree(p1);

    printf(" * Freeing second allocation\n");
    myfree(p2);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Two allocations p1 and p2.  Free p2 then p1. */
void test_2allocs_free21() {
    unsigned char *p1, *p2;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_2allocs_free21:\n");

    if (!do_2_allocs(400, &p1, 300, &p2))
        goto Done;

    printf(" * Freeing second allocation\n");
    myfree(p2);

    printf(" * Freeing first allocation\n");
    myfree(p1);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Helper function to allocate three regions of memory, and do a few basic
 * operations on them.  The function returns 1 if successful, 0 if a failure
 * is encountered.
 */
int do_3_allocs(int size1, unsigned char **pp1,
                int size2, unsigned char **pp2,
                int size3, unsigned char **pp3) {
    unsigned char *p1 = NULL, *p2 = NULL, *p3 = NULL;
    int rc = 1;

    p1 = alloc_trample(size1, 0xbb);
    if (p1 == NULL)
        goto Done;

    p2 = alloc_trample(size2, 0xcc);
    if (p2 == NULL)
        goto Done;

    p3 = alloc_trample(size3, 0xdd);
    if (p3 == NULL)
        goto Done;

    if (overlaps(p1, size1, p2, size2)) {
        printf("   ERROR:  p1 and p2 overlap!\n");
        rc = 0;  // Failure!
    }

    if (overlaps(p1, size1, p3, size3)) {
        printf("   ERROR:  p1 and p3 overlap!\n");
        rc = 0;  // Failure!
    }

    if (overlaps(p2, size2, p3, size3)) {
        printf("   ERROR:  p2 and p3 overlap!\n");
        rc = 0;  // Failure!
    }

Done:
    if (!p1 || !p2 || !p3)
        rc = 0;

    // Return the pointers to memory we allocated.
    *pp1 = p1;
    *pp2 = p2;
    *pp3 = p3;

    return rc;
}


/* Three allocations p1, p2 and p3.  Free p1, then p2, then p3. */
void test_3allocs_free123() {
    unsigned char *p1, *p2, *p3;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_3allocs_free123:\n");

    if (!do_3_allocs(400, &p1, 300, &p2, 500, &p3))
        goto Done;

    printf(" * Freeing first allocation\n");
    myfree(p1);

    printf(" * Freeing second allocation\n");
    myfree(p2);

    printf(" * Freeing third allocation\n");
    myfree(p3);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Three allocations p1, p2 and p3.  Free p3, then p2, then p1. */
void test_3allocs_free321() {
    unsigned char *p1, *p2, *p3;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_3allocs_free321:\n");

    if (!do_3_allocs(400, &p1, 300, &p2, 500, &p3))
        goto Done;

    printf(" * Freeing third allocation\n");
    myfree(p3);

    printf(" * Freeing second allocation\n");
    myfree(p2);

    printf(" * Freeing first allocation\n");
    myfree(p1);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


/* Three allocations p1, p2 and p3.  Free p1, then p3, then p2. */
void test_3allocs_free132() {
    unsigned char *p1, *p2, *p3;

    MEMORY_SIZE = POOL_SIZE;
    init_myalloc();

    printf("\ntest_3allocs_free132:\n");

    if (!do_3_allocs(400, &p1, 300, &p2, 500, &p3))
        goto Done;

    printf(" * Freeing first allocation\n");
    myfree(p1);

    printf(" * Freeing third allocation\n");
    myfree(p3);

    printf(" * Freeing second allocation\n");
    myfree(p2);

Done:
    printf(" * Done.\n");
    /* TODO:  uninit_myalloc(); */
}


int main(int argc, char *argv[]) {
    test_single_alloc();

    test_2allocs_free12();
    test_2allocs_free21();

    test_3allocs_free123();
    test_3allocs_free321();
    test_3allocs_free132();

    printf("\nAll done.\n");

    return 0;
}


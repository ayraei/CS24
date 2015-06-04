/*============================================================================
 * Implementation of the RANDOM page replacement policy.
 *
 * We don't mind if policies use malloc() and free(), just because it keeps
 * things simpler.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmpolicy.h"


/*============================================================================
 * Page-List Data Structure
 */


/* A single node in the page-info linked list. */
typedef struct pageinfo_t {
    page_t page;
    struct pageinfo_t *next;
} pageinfo_t;


/* A page-info linked list structure for tracking page details. */
typedef struct pagelist_t {
    pageinfo_t *head;
    pageinfo_t *tail;
} pagelist_t;


/* Find the pageinfo struct with the specified page number.  NULL is returned
 * if the page cannot be found.  If desired, the previous page can be returned
 * by this function as well.
 */
pageinfo_t * find_page(pagelist_t *list, page_t page, pageinfo_t **p_prev) {
    pageinfo_t *pginfo, *prev;

    assert(list != NULL);

    pginfo = list->head;
    prev = NULL;
    while (pginfo != NULL) {
        if (pginfo->page == page) {
            if (p_prev != NULL)
                *p_prev = prev;
            return pginfo;
        }
        prev = pginfo;
        pginfo = pginfo->next;
    }
    return NULL;
}


/* Add a pageinfo_t struct to the tail of the specified list. */
void add_to_tail(pagelist_t *list, pageinfo_t *pginfo) {
    assert(list != NULL);
    assert(pginfo != NULL);

    pginfo->next = NULL;

    if (list->tail != NULL)
        list->tail->next = pginfo;
    else
        list->head = pginfo;

    list->tail = pginfo;
}


/* Remove a pageinfo_t struct from the specified list.  The previous-node
 * pointer is necessary so that the linked list can be updated properly.  If
 * the node being removed is the first node in the list, the prev pointer
 * should be NULL (obvious).
 */
void remove_from_list(pagelist_t *list, pageinfo_t *pginfo, pageinfo_t *prev) {
    assert(prev == NULL || prev->next == pginfo);

    if (prev != NULL) {
        prev->next = pginfo->next;
    }
    else {
        assert(list->head == pginfo);
        list->head = pginfo->next;
    }

    if (pginfo == list->tail)
        list->tail = prev;

    pginfo->next = NULL;
}


/* Add the specified page to the end of the linked list.  This is a wrapper
 * for the add_to_tail() function.
 */
void add_page(pagelist_t *list, page_t page) {
    pageinfo_t *pginfo;

    assert(list != NULL);
    assert(find_page(list, page, NULL) == NULL);

    /* Always add the page to the back of the queue. */

    pginfo = malloc(sizeof(pageinfo_t));
    pginfo->page = page;
    pginfo->next = NULL;

    add_to_tail(list, pginfo);
}


/*============================================================================
 * Policy Implementation
 */


/* The list of pages that are currently resident. */
static pagelist_t pagelist;


/* Initialize the policy.  Return 0 for success, -1 for failure. */
int policy_init() {
    fprintf(stderr, "Using RANDOM eviction policy.\n\n");
    pagelist.head = NULL;
    pagelist.tail = NULL;
    return 0;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    add_page(&pagelist, page);
}


/* This function is called when the virtual memory system unmaps a page from
 * the virtual address space.  Remove the page from the list of resident
 * pages.
 */
void policy_page_unmapped(page_t page) {
    pageinfo_t *pginfo, *prev;

    pginfo = find_page(&pagelist, page, &prev);
    assert(pginfo != NULL);

    remove_from_list(&pagelist, pginfo, prev);
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick() {
    /* Do nothing! */
}


/* Choose a random page from the list of mapped pages, to evict.  Since we
 * use a linked list, use Reservoir Sampling to randomly select a page from
 * the list with a uniform probability.
 */
page_t choose_victim_page() {
    page_t victim;
    int i;
    pageinfo_t *pginfo;

    assert(pagelist.head != NULL);

    victim = pagelist.head->page;
    i = 2;
    pginfo = pagelist.head->next;
    while (pginfo != NULL) {
        if (rand() % i == 0)
            victim = pginfo->page;

        i++;
        pginfo = pginfo->next;
    }

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}


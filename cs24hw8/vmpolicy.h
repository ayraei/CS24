/*============================================================================
 * This file defines an interface that can be implemented to provide a page
 * replacement policy in the virtual memory system.
 *
 * We don't mind if policies use malloc() and free(), just because it keeps
 * things simpler.
 */

#ifndef VMPOLICY_H
#define VMPOLICY_H

#include "virtualmem.h"

/* Called by vmem_init() to initialize the page replacement policy.  The
 * function should return 0 for successful initialization, -1 otherwise.
 */
int policy_init();

/* Called by the virtual memory system to inform the policy when pages are
 * mapped into virtual memory, or unmapped from virtual memory.
 */
void policy_page_mapped(page_t page);
void policy_page_unmapped(page_t page);

/* Called by the virtual memory system when a SIGALRM signal is fired, so
 * that timer-based policies can update their internal state.
 */
void policy_timer_tick();

/* Called by map_page() when space needs to be made for another virtual page
 * to be mapped, by evicting a currently mapped page.  Note that this function
 * merely needs to choose a page to evict; the virtual memory system will call
 * policy_page_unmapped() with the victim page's number when it is actually
 * evicted.
 */
page_t choose_victim_page();


#endif /* VMPOLICY_H */


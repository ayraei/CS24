/*============================================================================
 * This file contains the implementation of the simple user-space virtual
 * memory system.
 */


#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "virtualmem.h"
#include "vmpolicy.h"


/* The start of the virtual address range.  Choosing a value for this is a bit
 * dangerous, because we could hit the memory heap (we are above it) or we
 * could hit shared libraries (we are below them).  However, our program's
 * heap requirements aren't very large, and this value seems to work
 * successfully.
 */
#define VIRTUALMEM_ADDR_START 0x20000000

/* The timer signal is set to trigger on this interval, currently 10ms. */
#define TIMESLICE_SEC 0
#define TIMESLICE_USEC 10000


/*============================================================================
 * Global state for the virtual memory system.
 *
 * (Don't do this at home.  At the least, this should all be wrapped up into
 * a single struct.)
 */

/* This is the address of where the virtual memory range starts. */
static void *vmem_start;

/* This is the address of where the virtual memroy range ends (it is actually
 * one past where virtual memory ends).
 */
static void *vmem_end;


/* The filename of the swap file. */
static char swapfile[40];

/* The file-descriptor of the swap file. */
static int fd_swapfile;


/* The number of pages that are currently resident. */
static unsigned int num_resident;


/* The maximum number of pages that may be resident in memory. */
static unsigned int max_resident;


/* A count of how many faults have occurred since initialization.  Note that
 * this doesn't correspond to the number of page-loads, since we use faults
 * to detect accesses and writes as well.
 */
static unsigned int num_faults;


/* A count of how many page-loads have occurred since initialization. */
static unsigned int num_loads;


/* This page table records the state of every virtual page in the virtual
 * memory area, including whether the page has been mapped into physical
 * memory, and also whether the page has been accessed and/or is dirty.
 */
static pte_t page_table[NUM_PAGES];


/*============================================================================
 * Helper Functions
 */


/* Returns the start of the virtual memory pool. */
void * get_vmem_start() {
    return vmem_start;
}


/* Returns the end of the virtual memory pool. */
void * get_vmem_end() {
    return vmem_end;
}


/* Takes a page number and returns the address of the start of the
 * corresponding virtual memory page.
 */
void * page_to_addr(page_t page) {
    assert(page < NUM_PAGES);
    return vmem_start + page * PAGE_SIZE;
}


/* Takes an address and returns the virtual memory page corresponding to
 * the address.
 */
page_t addr_to_page(void *addr) {
    assert(addr >= vmem_start);
    assert(addr < vmem_end);
    return (addr - vmem_start) / PAGE_SIZE;
}


/* Returns the number of segfaults that occurred in the system.  Note that
 * segfaults don't correspond to page-faults, because we use segfaults for
 * other things besides detecting page faults.  The number of page loads,
 * reported by the next function, is the best one to use to measure the system
 * performance.
 */
unsigned int get_num_faults() {
    return num_faults;
}


/* Returns the number of page-loads (i.e. map_page() calls) that have occurred
 * in the system.  This corresponds directly to the number of "page faults" in
 * the system.  This is the number we want to minimize.
 */
unsigned int get_num_loads() {
    return num_loads;
}


/* Returns a string representation of the signal-code value from the SIGSEGV
 * signal details.
 */
const char *signal_code(int code) {
    switch (code) {
    case SEGV_MAPERR:
        return "SEGV_MAPERR";
    case SEGV_ACCERR:
        return "SEGV_ACCERR";
    default:
        return "UNKNOWN";
    }
}


/*============================================================================
 * Page Table and Page Table Entry (PTE) Helper Functions
 */


/* This function should be used when a page is unmapped, since we want to
 * clear out all bits associated with the page's PTE.
 */
void clear_page_entry(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] = 0;
}


/* Sets the specified page's "resident" bit in its page-table entry. */
void set_page_resident(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] |= PAGE_RESIDENT;
}


/* Returns the specified page's "resident" bit.  Nonzero means the page is
 * present in memory, zero means the page is not in virtual memory.
 */
int is_page_resident(page_t page) {
    assert(page < NUM_PAGES);
    return page_table[page] & PAGE_RESIDENT;
}


/* Sets the specified page's "accessed" bit in its page-table entry. */
void set_page_accessed(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] |= PAGE_ACCESSED;
}


/* Clears the specified page's "accessed" bit in its page-table entry. */
void clear_page_accessed(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] &= ~PAGE_ACCESSED;
}


/* Returns the specified page's "accessed" bit.  Nonzero means the page has
 * been accessed, zero means the page has not been accessed.
 */
int is_page_accessed(page_t page) {
    assert(page < NUM_PAGES);
    return page_table[page] & PAGE_ACCESSED;
}


/* Sets the specified page's "dirty" bit in its page-table entry. */
void set_page_dirty(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] |= PAGE_DIRTY;
}


/* Clears the specified page's "dirty" bit in its page-table entry. */
void clear_page_dirty(page_t page) {
    assert(page < NUM_PAGES);
    page_table[page] &= ~PAGE_DIRTY;
}


/* Returns the specified page's "dirty" bit.  Nonzero means the page has
 * been written to, zero means the page has not been written to.
 */
int is_page_dirty(page_t page) {
    assert(page < NUM_PAGES);
    return page_table[page] & PAGE_DIRTY;
}


/* Returns the specified page's permission value from the page-table entry.
 * The other bits (e.g. resident, accessed, dirty) are masked out of this
 * return-value.
 */
int get_page_permission(page_t page) {
    assert(page < NUM_PAGES);
    return page_table[page] & PAGEPERM_MASK;
}


/* Sets the specified page's permission value.  This involves two steps:
 * First, mprotect() is used to set the actual permissions on the page's
 * virtual address range.  After this is completed successfully, the page's
 * Page Table Entry is updated with the new permission value.  (The other bits
 * in the PTE are left unmodified.)
 */
void set_page_permission(page_t page, int perm) {
    assert(page < NUM_PAGES);
    assert(perm == PAGEPERM_NONE || perm == PAGEPERM_READ ||
           perm == PAGEPERM_RDWR);

    /* Call mprotect() to set the memory region's protections. */
    if (mprotect(page_to_addr(page), PAGE_SIZE, pageperm_to_mmap(perm)) == -1) {
        perror("mprotect");
        abort();
    }

    /* Replace old permission with new permission. */
    page_table[page] = (page_table[page] & ~PAGEPERM_MASK) | perm;
}


/* This function converts a page table entry's permission value into the
 * corresponding value to pass to mmap() or mprotect().
 */
int pageperm_to_mmap(int perm) {
    assert(perm == PAGEPERM_NONE || perm == PAGEPERM_READ ||
           perm == PAGEPERM_RDWR);

    switch (perm) {
    case PAGEPERM_NONE:
        return PROT_NONE;

    case PAGEPERM_READ:
        return PROT_READ;

    case PAGEPERM_RDWR:
        return PROT_READ | PROT_WRITE;

    default:
        fprintf(stderr, "pageperm_to_mmap: unrecognized value %u\n", perm);
        abort();
    }
}


/*============================================================================
 * Core Functions for the User-Space Virtual Memory System
 */

/* Declare these functions here, since they really aren't needed outside of
 * the virtual-memory code itself.
 */
void map_page(page_t page, unsigned initial_perm);
void unmap_page(page_t page);
static void sigsegv_handler(int signum, siginfo_t *infop, void *data);
static void sigalrm_handler(int signum, siginfo_t *infop, void *data);


/* This function initializes the virtual memory system with the specified
 * "maximum resident" limit on the number of pages that may be in the address
 * space.  This function does the following:
 *
 * 1)  Set aside a range of addresses for use by the user-space virtual memory
 *     system.
 *
 * 2)  Initialize other virtual memory system state.
 *
 * 3)  Clear the entire Page Table to all 0s.
 *
 * 4)  Initialize the page replacement policy.
 *
 * 5)  Open the swap file /tmp/cs24_pagedev_<pid>, extend it to be the full
 *     size of the virtual address space (NUM_PAGES * PAGE_SIZE), and arrange
 *     for the file to be deleted when the program terminates.
 *
 * 6)  Install the SIGSEGV and SIGALRM handlers.
 *
 * 7)  Start the SIGALRM timer interrupt.
 */
void * vmem_init(unsigned _max_resident) {
    struct sigaction action;
    struct itimerval itimer;

    /* Set up the address range we will use. */
    vmem_start = (void *) VIRTUALMEM_ADDR_START;
    vmem_end = vmem_start + (NUM_PAGES * PAGE_SIZE);
    fprintf(stderr, "\"Physical memory\" is in the range %p..%p\n * %d pages"
            " total, %d maximum resident pages\n", vmem_start, vmem_end,
            NUM_PAGES, max_resident);

    /* Initialize the values that record how many pages are resident in
     * physical memory, and the maximum number of pages that may be resident.
     */
    num_resident = 0;
    max_resident = _max_resident;
    num_faults = 0;

    /* Clear the entire page table. */
    memset(page_table, 0, sizeof(page_table));

    /* Initialize the page replacement policy. */
    if (policy_init() == -1) {
        fprintf(stderr, "policy_init: failed to initialize\n");
        abort();
    }

    /* Open the swap file */
    sprintf(swapfile, "/tmp/cs24_pagedev_%05d", getpid());
    fd_swapfile = open(swapfile, O_RDWR | O_CREAT, 0600);
    if (fd_swapfile < 0) {
        perror(swapfile);
        abort();
    }

    /* Immediately unlink it so it will go away when this process terminates */
    if (unlink(swapfile) < 0) {
        perror(swapfile);
        abort();
    }

    /* Extend the file to include the entire address space */

    if (lseek(fd_swapfile, NUM_PAGES * PAGE_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        abort();
    }

    if (write(fd_swapfile, "x", 1) < 1) {
        perror(swapfile);
        abort();
    }

    /* Set up and install the seg-fault signal handler */

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = sigsegv_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;

    /* Mask timer signals during the SIGSEGV handler */
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGALRM);

    if (sigaction(SIGSEGV, &action, (struct sigaction *) 0) < 0) {
        perror("sigaction(SIGSEGV)");
        exit(1);
    }

    /* Set up and install the timer signal handler */

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = sigalrm_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    if (sigaction(SIGALRM, &action, (struct sigaction *) 0) < 0) {
        perror("sigaction(SIGALRM)");
        exit(1);
    }

    /* Start the periodic timer! */
    itimer.it_interval.tv_sec = TIMESLICE_SEC;
    itimer.it_interval.tv_usec = TIMESLICE_USEC;
    itimer.it_value.tv_sec = TIMESLICE_SEC;
    itimer.it_value.tv_usec = TIMESLICE_USEC;
    if (setitimer(ITIMER_REAL, &itimer, (struct itimerval *) 0) < 0) {
        perror("setitimer");
        exit(1);
    }

    return vmem_start;
}


/* This function maps the specified page from the swap file into the virtual
 * address space, and sets up the page permissions so that accesses and writes
 * to the page can be detected.
 */
void map_page(page_t page, unsigned initial_perm) {
    assert(page < NUM_PAGES);
    assert(initial_perm == PAGEPERM_NONE || initial_perm == PAGEPERM_READ ||
           initial_perm == PAGEPERM_RDWR);
    assert(!is_page_resident(page));  /* Shouldn't already be mapped */

#if VERBOSE
    fprintf(stderr, "Mapping in page %u.  Resident (before mapping) = %u, "
           "max resident = %u.\n", page, num_resident, max_resident);
#endif

    /* Make sure we don't exceed the physical memory constraint. */
    num_resident++;
    if (num_resident > max_resident) {
        fprintf(stderr, "map_page: exceeded physical memory, resident pages "
                "= %u, max resident = %u\n", num_resident, max_resident);
        abort();
    }

    /* Use mmap() to add the page's address-range to the process' virtual
     * memory.
     */
    void *addr = mmap(page_to_addr(page),
        PAGE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0);
    /* Detect and report an error with mmap(). */
    if (addr == (void *) -1) {
        perror("mmap");
        abort();
    }
    if (addr != page_to_addr(page)) {
        fprintf(stderr, "mmap: expected address %p; got %p\n",
            page_to_addr(page), addr);
        abort();
    }
    
    /* Seek to the start of the corresponding slot in the swap file and
     * read the slot's contents into the page mapped. Detects and reports
     * any error with lseek().
     */
    if (lseek(fd_swapfile, page * PAGE_SIZE, SEEK_SET) == -1) {
        perror("lseek");
        abort();
    }
    /* Detect and report an error with read(). */
    int rc =  read(fd_swapfile, page_to_addr(page), PAGE_SIZE);
    if (rc == -1) {
        perror("read");
        abort();
    }
    if (rc != PAGE_SIZE) {
        fprintf(stderr, "read: only read %d bytes (%d expected)\n",
            rc, PAGE_SIZE);
        abort();
    }
    
    /* Update the Page Table Entry. */
    set_page_resident(page);
    set_page_permission(page, initial_perm);

    assert(is_page_resident(page));  /* Now it should be mapped! */
    num_loads++;

    /* Inform the paging policy that the page was mapped. */
    policy_page_mapped(page);

#if VERBOSE
    fprintf(stderr, "Successfully mapped in page %u with initial "
        "permission %u.\n  Resident (after mapping) = %u.\n",
        page, initial_perm, num_resident);
#endif
}


/* This function unmaps the specified page from the virtual address space,
 * making sure to write the contents of dirty pages back into the swap file.
 */
void unmap_page(page_t page) {
    assert(page < NUM_PAGES);
    assert(num_resident > 0);
    assert(is_page_resident(page));
    
    /* Check if the page is dirty, and if so, save it back to the swapfile. */
    if (is_page_dirty(page)) {
        /* Make the page accessible. */
        set_page_permission(page, PAGEPERM_READ);
        
        /* Seek to the appropriate slot in the swapfile and detect any
         * error from lseek().
         */
        if (lseek(fd_swapfile, page * PAGE_SIZE, SEEK_SET) == -1) {
            perror("lseek");
            abort();
        }
        
        /* Write the page back to swapfile; detect any error from write(). */
        int w = write(fd_swapfile, page_to_addr(page), PAGE_SIZE);
        if (w == -1) {
            perror("write");
            abort();
        }
        if (w != PAGE_SIZE) {
            fprintf(stderr, "write: only wrote %d bytes (%d expected)\n",
                w, PAGE_SIZE);
            abort();
        }
    }
    
    /* Unmap the page and detect any error with munmap(). */
    int ret = munmap(page_to_addr(page), PAGE_SIZE);
    if (ret == -1) {
        perror("munmap");
        abort();
    }
    
    /* Clear the Page Table Entry. */
    clear_page_entry(page);

    assert(!is_page_resident(page));
    num_resident--;

    /* Inform the paging policy that the page was unmapped. */
    policy_page_unmapped(page);
}


/*============================================================================
 * Signal Handlers for the Virtual Memory System
 */


/* This function is the SIGSEGV handler for the virtual memory system.  If the
 * faulting address is within the user-space virtual memory pool then this
 * function responds appropriately to allow the faulting operation to be
 * retried.  If the faulting address falls outside of the virtual memory pool
 * then the segmentation fault is reported as an error.
 *
 * While a SIGSEGV is being processed, SIGALRM signals are blocked.  Therefore
 * a timer interrupt will never interrupt the segmentation-fault handler.
 */
static void sigsegv_handler(int signum, siginfo_t *infop, void *data) {
    void *addr;
    page_t page;

    /* Only handle SIGSEGVs addresses in range */
    addr = infop->si_addr;
    if (addr < vmem_start || addr >= vmem_end) {
        fprintf(stderr, "segmentation fault at address %p\n", addr);
        abort();
    }

    num_faults++;

    /* Figure out what page generated the fault. */
    page = addr_to_page(addr);
    assert(page < NUM_PAGES);

#if VERBOSE
    fprintf(stderr,
        "================================================================\n");
    fprintf(stderr, "SIGSEGV:  Address %p, Page %u, Code %s (%d)\n",
           addr, page, signal_code(infop->si_code), infop->si_code);
#endif

    /* We really can't handle any other type of code.  On Linux this should be
     * fine though.
     */
    assert(infop->si_code == SEGV_MAPERR || infop->si_code == SEGV_ACCERR);

    /* Map the page into memory so that the fault can be resolved.  Of course,
     * this may result in some other page being unmapped.
     */
    
    /* If the page is unmapped (SEGV_MAPERR), map the corresponding page
     * into memory. Evict a page if there are already max_resident pages
     * loaded.
     */
    if (infop->si_code == SEGV_MAPERR) {
        assert(num_resident <= max_resident);
        /* Evict a page. */
        if (num_resident == max_resident) {
            page_t victim = choose_victim_page();
            assert(is_page_resident(victim));
            unmap_page(victim);
            assert(!is_page_resident(victim));
        }
        /* Now we can map the page we want. */
        map_page(page, PAGEPERM_NONE);
    }
    
    /* The address is mapped, but an access violation occurred (SEGV_ACCERR).
     * If the page started with no access permissions, give it read permission
     * and set PTE to accessed. If the page started with read permission,
     * give it read/write permission and set PTE to dirty.
     */
    else if (infop->si_code == SEGV_ACCERR) {
        int p = get_page_permission(page);
        /* Page access. */
        if (p == PAGEPERM_NONE) {
            set_page_permission(page, PAGEPERM_READ);
            set_page_accessed(page);
        }
        /* Page write. */
        else if (p == PAGEPERM_READ) {
            set_page_permission(page, PAGEPERM_RDWR);
            set_page_dirty(page);
        }
        /* We can't give the page any more permissions... */
        else {
            fprintf(stderr, "received SEGV_ACCERR and unsure how\n");
            abort();
        }
    }
    
    /* This should be unreachable because of the assertion from earlier! */
    else {
        fprintf(stderr, "segfaulted with invalid code\n");
        abort();
    }
}


/* This function is the SIGALRM handler for the virtual memory system.  On a
 * timer interrupt, call the paging policy timer function.
 *
 * While a SIGSEGV is being processed, SIGALRM signals are blocked.  Therefore
 * a timer interrupt will never interrupt the segmentation-fault handler.
 */
static void sigalrm_handler(int signum, siginfo_t *infop, void *data) {
#if VERBOSE
    fprintf(stderr,
        "================================================================\n");
    fprintf(stderr, "SIGALRM\n");
#endif

    /* All we have to do is inform the page replacement policy that a timer
     * tick occurred!
     */
    policy_timer_tick();
}



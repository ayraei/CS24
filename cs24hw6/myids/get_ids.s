.global get_ids

get_ids:
	pushl %ebp          /* Push old base pointer. */
	movl %esp,%ebp      /* Current stack is new base. */

    /* Get the User ID. */
    movl $24, %eax      /* Set eax = 24 (which is getuid). */
    int $0x80           /* Invoke the system call. */
    
    /* Now, the User ID is stored in eax, which we need to store to uid. */
    movl 8(%ebp), %edx
    movl %eax, (%edx)
    
    /* Now, get the Group ID. */
    movl $47, %eax      /* Set eax = 47, which is getgid. */
    int $0x80           /* Invoke system call. */
    
    /* Group ID is returned in eax, which we want to store to gid. */
    movl 12(%ebp), %edx
    movl %eax, (%edx)

    /* Return. */
	movl %ebp,%esp      /* Pop local stack. */
	popl %ebp           /* Pop old base of frame. */
	ret

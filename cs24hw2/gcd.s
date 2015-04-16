.globl gcd

gcd:
	pushl %ebp          /* Push old base pointer. */
	movl %esp,%ebp      /* Current stack is new base. */
	movl 8(%ebp),%eax   /* Load x into eax. */
	cltd                /* edx:eax = x */
	idivl 12(%ebp)      /* Compute x / y. eax = quotient, edx = remainder */
	cmpl $0,%edx        /* See if remainder equals zero. */
	jne gcd_continue    /* If not, continue. */
	movl 12(%ebp),%eax  /* Else load the GCD into eax... */
	jmp gcd_return      /* ...and we can finish. */

gcd_continue:
	pushl %edx          /* The second argument. */
	pushl 12(%ebp)      /* The first argument. */
	call gcd            /* Recursive call to gcd. */

gcd_return:
	movl %ebp,%esp      /* Pop local stack. */
	popl %ebp           /* Pop old base of frame. */
	ret

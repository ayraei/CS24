.globl my_setjmp

my_setjmp:
    mov 4(%esp), %eax        /* Get jmp_buf location in memory */
    
    mov %ebx, (%eax)  /* Store ebx (callee-save) */
    mov %esi, 1(%eax)  /* Store esi (callee-save) */
    mov %edi, 2(%eax)  /* Store edi (callee-save) */
    mov %ebp, 3(%eax)  /* Store base pointer */
    mov %esp, 4(%eax)  /* Store stack pointer */
    mov 8(%esp), %ecx    /* Store caller return address. */
    mov %ecx, 5(%eax)
    
    mov $0, %eax        /* Always return 0. */
    
    ret
    

.globl my_longjmp

my_longjmp:
	mov %esp,%ebp      /* Current stack is new base. */
    
    mov 4(%esp), %edx       /* Start of jmp_buf in memory */
    mov 8(%esp), %eax      /* return value */
    
    test %eax, %eax         /* check if zero */
    jnz ret_nonzero
    mov $1, %eax
    
ret_nonzero:
    mov (%edx), %ebx  /* Load ebx (callee-save) */
    mov 1(%edx), %esi  /* Load esi (callee-save) */
    mov 2(%edx), %edi  /* Load edi (callee-save) */
    mov 3(%edx), %ebp  /* Load base pointer */
    mov 4(%edx), %esp  /* Load stack pointer */
    inc %esp
    push 5(%edx)

    ret

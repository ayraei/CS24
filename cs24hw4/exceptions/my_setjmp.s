.globl my_setjmp

my_setjmp:
    mov 8(%ebp), %eax        /* Get jmp_buf location in memory */
    
    xor %ecx, %ecx          /* Zero out ecx */
    mov %ebx, (%eax, %ecx)  /* Store ebx (callee-save) */
    inc %ecx                /* We increment ecx so we don't hard-code values. */
    mov %esi, (%eax, %ecx)  /* Store esi (callee-save) */
    inc %ecx
    mov %edi, (%eax, %ecx)  /* Store edi (callee-save) */
    inc %ecx
    mov %esp, (%eax, %ecx)  /* Store stack pointer */
    inc %ecx
    mov %ebp, (%eax, %ecx)  /* Store base pointer */
    inc %ecx
    mov 4(%ebp), (%eax, %ecx)   /* Store return address */
    

.globl my_longjmp

my_longjmp:
    push %ebp
    mov %esp, %ebp
    
    mov 8(%ebp), %edx       /* Start of jmp_buf in memory */
    mov 12(%ebp), %eax      /* return value */
    
    test %eax, %eax         /* check if zero */
    jnz ret_nonzero
    mov $1, %eax
    
ret_nonzero:
    xor %ecx, %ecx      /* Zero out ecx */
    mov (%eax, %ecx), %
    inc %ecx

.globl draw_pixel

draw_pixel:
	push %ebp           /* Push old base pointer. */
	mov %esp, %ebp      /* Current stack is new base. */
	push %ebx
	push %esi
	push %edi
	mov 8(%ebp), %ecx   /* Store the Screen pointer somewhere. */
	
	/* First check if the coordinate given is out of bounds. */
	mov (%ecx), %eax    /* %eax holds Screen width */
	mov 4(%ecx), %ebx   /* %ebx holds Screen height */
	
	cmp $0, 12(%ebp)    /* Check if x < 0 */
	jle done
	cmp %eax, 12(%ebp)  /* Check if x >= w */
	jge done
	cmp $0, 16(%ebp)    /* Check if y < 0 */
	jle done
	cmp %ebx, 16(%ebp)  /* Check if y >= h */
	jge done
	jmp valid

done:
    /* Restore callee-save registers and do the usual stack cleanup. */
    pop %edi
    pop %esi
    pop %ebx
    mov %ebp, %esp
    pop %ebp
    ret

valid:
    /* Compute location of Pixel[x, y]. */
    mov 16(%ebp), %esi      /* %esi holds the Pixel coordinate */
    imul %eax, %esi         /* currently coord = y*w */
    add 12(%ebp), %esi      /* coord = y*w + x */
    
    /* Compute its byte offset from the beginning of the Screen struct. */
    imul $2, %esi           /* Now %esi holds the byte offset */
    add $8, %esi            /* loc = 8 + 2*coord */

    /* Check depth. */
    mov 1(%ecx, %esi), %edi /* %edi holds d = Screen[loc + 1] */
    cmp 21(%ebp), %edi      /* Check if d >= depth */
    jge draw                /* If yes, draw the pixel */
    jmp done                /* Otherwise, ignore and return */

draw:
    mov 20(%ebp), %eax      /* temporarily store 'value' argument */
    mov %eax, (%ecx, %esi)  /* Screen[loc] = value */

	.file	"example.c"
	.text
	.globl	ex
	.type	ex, @function
ex:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$16, %esp           # Subtraction step: b - c
	movl	12(%ebp), %eax
	subl	16(%ebp), %eax
	imull	8(%ebp), %eax       # Multiplication step: a * (b - c)
	movl	%eax, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax          # Addition step: a * (b - c) + d
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	ex, .-ex
	.ident	"GCC: (GNU) 4.9.2 20150304 (prerelease)"
	.section	.note.GNU-stack,"",@progbits

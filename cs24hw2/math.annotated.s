/* This file contains IA32 assembly-language implementations of three
 * basic, very common math operations.
 *
 * The naive approach to implementing these three functions is to
 * check some property of the number/s (e.g. "if x < y" or
 * "if x < 0"), then set the result to be one value if the conditional
 * evaluated to true, or a different value if it evaluated to false.
 * This would require a jump operation in each function. According to
 * the textbook, conditional data transfers (e.g. cmovg) tend to
 * outperform conditional control transfers (e.g. jumps), so
 * implementing these functions the naive way would reduce performance.
 *
 */

    .text

/*====================================================================
 * This function returns min(x, y).
 *
 * int f1(int x, int y)
 */
.globl f1
f1:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx       # load y into edx
	movl	12(%ebp), %eax      # load x into eax
	cmpl	%edx, %eax          # sets flag for x - y
	cmovg	%edx, %eax          # if x - y > 0, move y into eax
	popl	%ebp                #   i.e. if y < x, answer is y; else x.
	ret


/*====================================================================
 * This function returns abs(x).
 * If x starts as a negative value, this function converts using the
 * two's-complement conversion trick of flipping all the bits
 * and adding 1.
 *
 * int f2(int x)
 */
.globl f2
f2:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax   # Load x into eax
	movl	%eax, %edx      # Copy x into edx
	sarl	$31, %edx       # edx is now 0xffff if x<0 or 0x0000 if x>=0
	xorl	%edx, %eax      # negates eax if x<0, leave eax unchanged if x>=0
	subl	%edx, %eax      # equiv. to adding 1 if x<0, eax unchanged if x>=0
	popl	%ebp
	ret


/*====================================================================
 * This function returns sign(x).
 *
 * int f3(int x)
 */
.globl f3
f3:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx   # Load x into edx
	movl	%edx, %eax      # Copy x into eax
	sarl	$31, %eax       # eax is 0xffff if x<0; eax is 0x0000 if x>=0
	testl	%edx, %edx      # set flag for sign of x
	movl	$1, %edx        # Stores 1 into edx
	cmovg	%edx, %eax      # transfers 1 into result (eax) if x>0
	popl	%ebp            # otherwise eax contains 0xffff if x<0
	ret                     # eax contains 0x0000 if x=0


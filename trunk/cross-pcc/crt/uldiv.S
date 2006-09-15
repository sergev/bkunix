/*
 * Program: uldiv.s
 * Copyright 1993, GTE Government Systems
 * Author:  Steven M. Schultz
 *
 *  Version	Date		Modification
 *	0.0	02Feb91		1. Initial inspiration struck.
 *	1.0	05Jun93		2. Released into the Public Domain.
*/

#include "DEFS.h"

/*
 * All routines have both a C interface and an assembly interface.  Normally
 * the two are the same.  In the case of 'ulsh' the compiler has placed one
 * of the operands in r0 and r1 so the assembly interface differs from the
 * C interface.
*/

/*
 * u_long uldiv(lhs, rhs)
 *	u_long	lhs, rhs;
 *
 * unsigned 32-bit "/" routine.  Calls to uldiv are generated automatically 
 * by the C compiler.
 */

#if !defined(KERNEL)
/*
 * uldiv for applications (uses floating point)
 */
	.globl l2f, l6f
	.globl uldiv
uldiv:
ENTRY(uldiv)
	jsr	pc,l2f		/ 2(sp) -> fr0
	jsr	pc,l6f		/ 6(sp) -> fr3
	tstf	fr3		/ check for zero divisor
	cfcc			/   don't want to have an FP fault
	beq	1f		/   in integer arithmetic
	divf	fr3,fr0		/ fr0 /= rhs
1:
	movfi	fr0,-(sp)
	mov	(sp)+,r0	/ return result
	mov	(sp)+,r1
	seti
	rts	pc
#else
/*
 * uldiv for the kernel (fixed point only - no FP)
 */

	.globl uldiv
uldiv:
ENTRY(uldiv)
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	14.(sp),r3	/ r3 = lo(rhs)
	bmi	slowuldiv	/  rhs >= 2^15
	tst	12.(sp)		/ hi(rhs) empty?
	bne	slowuldiv	/   no, rhs >= 2^16

	mov	10.(sp),r2	/ r2 = lo(lhs)
	mov	8.(sp),r1	/ r1 = hi(lhs)

	clr	r0		/ r0 = hi(lhs) / lo(rhs)
	div	r3,r0		/ r1 = hi(lhs) % lo(rhs)
	mov	r0,r4		/ save high quotient
	mov	r1,-(sp)	/ stash hi(tmp)
	mov	r1,r0		/ tmp=(hi(lhs)%lo(rhs))<<16 | lo(lhs)
	mov	r2,r1		/ (r0:r1 = tmp)
	div	r3,r0		/ r0 = tmp / lo(rhs)
	bvc	3f		/ done if tmp/lo(rhs) < 2^15

	mov	(sp),r0		/ reload r0:r1 with tmp (regs may be
	mov	r2,r1		/   clobbered by failed div)
	sub	r3,r0		/ r0:r1 -= 2^16 * lo(rhs)
	div	r3,r0
	tst	r1		/ if (negative) remainder, subtract one from
	sxt	r1		/   quotient
	add	r1,r0		/ cannot overflow!
3:
	tst	(sp)+		/ pop hi(tmp) off stack
	mov	r0,r1		/ r1 (lo(quo)) = tmp / lo(rhs)
	mov	r4,r0		/ r0 (hi(quo)) = hi(lhs) / lo(rhs)
9:
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/*
 * The divisor (rhs) is known to be >= 2^15 so we perform a shift and
 * subtract algorithm.  It's slow - feel free to improve it.
 *
 * The algorithm for signed divide broke down for unsigned operands, a slower
 * larger, more painful algorithm was implmented using scaling and
 * repetitive subraction/shifting.  Works best for large numbers (fewer
 * shifts that way).
 */
slowuldiv:
	mov	8.(sp),r0	/ r0 = hi(lhs)
	mov	10.(sp),r1	/ r1 = lo(lhs)
	mov	12.(sp),r2	/ r2 = hi(rhs)
				/ r3 = lo(rhs) - already done

	clr	r4		/ init scale of lhs
2:
	ashc	$1,r0
	blos	1f		/ check for zero at same time
	inc	r4
	br	2b
1:
	mov	r4,-(sp)	/ save scale of lhs
	clr	r4
2:
	asl	r3
	rol	r2
	bcs	1f
	inc	r4		/ bump rhs scale
	br	2b
1:
	clr	r0
	mov	$1,r1
	sub	(sp)+,r4	/ difference in scale (rhs - lhs)
	ashc	r4,r0		/ initial quotient adder
	mov	r1,-(sp)	/ quoadder lo
	mov	r0,-(sp)	/ quoadder hi
	mov	12.(sp),r0	/ r0 = hi(lhs)
	mov	14.(sp),r1	/ r1 = lo(lhs)
	mov	16.(sp),r2	/ r2 = hi(rhs)
	mov	18.(sp),r3	/ r3 = lo(rhs)

	ashc	r4,r2		/ scale rhs up for repetitive subtraction
	clr	r4		/ quo lo
	clr	-(sp)		/ quo hi
docmp:
	cmp	r2,r0
	bhi	noadd
	blo	dosub
	cmp	r3,r1
	bhi	noadd
dosub:
	sub	r3,r1
	sbc	r0
	sub	r2,r0
	add	4(sp),r4	/ quo lo += quoadder lo
	adc	(sp)		/ quo hi
	add	2(sp),(sp)	/ quo hi += quoadder hi
	br	docmp
noadd:
	clc			/ right shift rhs
	ror	r2
	ror	r3
	clc			/ right shift quotient adder
	ror	2(sp)
	ror	4(sp)
	bne	docmp		/ quo adder not 0 means more to do
	tst	2(sp)		
	bne	docmp
	mov	(sp)+,r0	/ quo hi
	mov	r4,r1		/ quo lo
	cmp	(sp)+,(sp)+	/ remove quot adder
	br	9b
#endif KERNEL

/*
 * u_long ualdiv(lhs, rhs)
 *	u_long	*lhs, rhs;
 *
 * 32-bit "/=" routine.  Calls to ualdiv are generated automatically by the C
 * compiler.
 */

	.globl	ualdiv
ualdiv:
ENTRY(ualdiv)
	mov	r2,-(sp)	/ need a register to point at the lhs
	mov	8.(sp),-(sp)	/ The divide algorithm is long
	mov	8.(sp),-(sp)	/   enough that it just doesn't make sense
	mov	8.(sp),r2	/   to bother repeating it.  We just translate
	mov	2(r2),-(sp)	/   the call for uldiv and let it do the work
	mov	(r2),-(sp)	/   and return its results (also stuffing it
	jsr	pc,uldiv	/   into *lhs)
	add	$8.,sp		/ clean up stack
	mov	r0,(r2)+	/ store high word,
	mov	r1,(r2)		/   and low
	mov	(sp)+,r2	/ restore r2
	rts	pc		/   and return

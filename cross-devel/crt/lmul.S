/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * lmul(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "*" routine for hardware without mul instruction.
 * Calls to aldiv are generated automatically by the C compiler.
 */
#define	negl(high, low)	neg	high; \
			neg	low; \
			sbc	high	/ high -= (low != 0)

	.globl	lmul
lmul:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov 	8(sp),r2	/ lhs high
	mov 	10(sp),r3	/ lhs low
	clr	r4		/ sign bit

	tst	12(sp)		/ set up flags for proper sign management
	bge	1f
	negl(12(sp),14(sp))	/ negate rhs
	inc	r4
1:
	tst	r2
	bge	1f
	negl(r2,r3)		/ negate lhs
	dec	r4
1:
	clr	r0
	clr	r1
	mov	$33,-(sp)	/ set counter for 33 cycles
2:
	clc			/ use shift and add algorithm
	ror	r1
	ror	r0
	ror	r3
	ror	r2
	bcc	1f
	add	16(sp),r1	/ add rhs low...
	adc	r0
	add	14(sp),r0	/ ...and high
1:
	dec	(sp)
	bne	2b

	cmp	r4,(sp)+	/ were signs the same?
	beq	1f

	neg	r0		/ no, negate the product
	neg	r1
	sbc	r0
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc		/ exit

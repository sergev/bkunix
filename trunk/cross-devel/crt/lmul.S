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
	mov 	8(sp),r0	/ lhs high
	mov 	10(sp),r1	/ lhs low
	clr	r4		/ sign bit

	tst	12(sp)		/ set up flags for proper sign management
	bge	1f
	negl(12(sp),14(sp))	/ negate rhs
	inc	r4
1:
	tst	r0
	bge	1f
	negl(r0,r1)		/ negate lhs
	dec	r4
1:
	clr	r2
	clr	r3
	mov	$33,-(sp)	/ set counter for 33 cycles
2:
	clc			/ use shift and add algorithm
	ror	r2
	ror	r3
	ror	r0
	ror	r1
	bcc	1f
	add	16(sp),r3	/ add rhs low...
	adc	r2
	add	14(sp),r2	/ ...and high
1:
	dec	(sp)
	bne	2b

	cmp	r4,(sp)+	/ were signs the same?
	beq	1f
	negl(r0,r1)		/ no, negate the product
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc		/ exit

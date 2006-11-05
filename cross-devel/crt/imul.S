/*
 * imul - 16-bit "*" routine for fixed point hardware.
 * Calls to imul are generated automatically by the C compiler.
 * Usage:
 * 	mov	lhs, r1
 * 	mov	rhs, r0
 *	jsr	pc, imul
 * 	mov	r0, result_high
 * 	mov	r1, result_low
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	imul
imul:
	mov	r2,-(sp)
	mov	r3,-(sp)
	clr	r3		/ sign bit

	mov	r1,r2		/ set up flags for proper sign management
	bge	1f
	neg	r2
	inc	r3
1:
	mov	r0,r1
	bge	1f
	neg	r1
	dec	r3
1:
	clr	r0
	mov	$17,-(sp)	/ set counter for 17 cycles
2:
	clc			/ use shift and add algorithm
	ror	r0
	ror	r1
	bcc	1f
	add	r2,r0
1:
	dec	(sp)
	bne	2b

	cmp	r3,(sp)+	/ were signs the same?
	beq	1f

	neg	r0		/ no, negate the product
	neg	r1
	sbc	r0
1:
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc		/ exit

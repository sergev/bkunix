/*
 * lshr - 32-bit ">>" routine.
 * Calls to lshr are generated automatically by the C compiler.
 *
 * 	mov	value_high, r0
 * 	mov	value_low, r1
 * 	mov	count, (sp)
 *	jsr	pc, lshr
 * 	mov	r0, result_high
 * 	mov	r1, result_low
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	lshr
lshr:
	bic	$!037,(sp)
	beq	9f
1:
	asr	r0		/ right shift
	ror	r1

	dec	(sp)
	bgt	1b
9:
	rts	pc		/ exit

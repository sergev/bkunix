/*
 * lshl - 32-bit "<<" routine.
 * Calls to lshl are generated automatically by the C compiler.
 *
 * 	mov	value_high, r0
 * 	mov	value_low, r1
 * 	mov	count, (sp)
 *	jsr	pc, lshl
 * 	mov	r0, result_high
 * 	mov	r1, result_low
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	lshl
lshl:
	bic	$!037,(sp)
	beq	9f
1:
	asl	r1		/ left shift
	rol	r0

	dec	(sp)
	bgt	1b
9:
	rts	pc		/ exit

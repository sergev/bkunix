/*
 * ishl - 16-bit "<<" routine.
 * Calls to ishl are generated automatically by the C compiler.
 *
 * 	mov	value, r0
 * 	mov	count, (sp)
 *	jsr	pc, ishl
 * 	mov	r0, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	ishl
ishl:
	bic	$!017,(sp)
	beq	9f
1:
	asl	r0		/ left shift

	dec	(sp)
	bgt	1b
9:
	rts	pc		/ exit

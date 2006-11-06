/*
 * ishl - 16-bit "<<" routine.
 * Calls to ishl are generated automatically by the C compiler.
 *
 * 	mov	value, r0
 * 	mov	count, -(sp)
 *	jsr	pc, ishl
 * 	mov	r0, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	ishl
ishl:
	mov	r2,-(sp)
	mov	4(sp),r2
	bic	$!017,r2
	beq	9f
1:
	asl	r0		/ left shift
	sob	r2,1b
9:
	mov	(sp)+,r2
	mov	(sp)+,(sp)
	rts	pc		/ exit

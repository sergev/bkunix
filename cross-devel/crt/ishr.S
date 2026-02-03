/*
 * ishr - 16-bit ">>" routine.
 * Calls to ishr are generated automatically by the C compiler.
 *
 * 	mov	value, r0
 * 	mov	count, -(sp)
 *	jsr	pc, ishr
 * 	mov	r0, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
	.globl	ishr
ishr:
	mov	r2,-(sp)
	mov	4(sp),r2
	bic	$!017,r2
	beq	9f
1:
	clc
	ror	r0		/ right shift, no sign extension
	sob	r2,1b
9:
	mov	(sp)+,r2
	mov	(sp)+,(sp)
	rts	pc		/ exit

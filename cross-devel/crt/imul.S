/*
 * imul - 16-bit "*" routine for fixed point hardware.
 * Calls to imul are generated automatically by the C compiler.
 * Usage:
 * 	mov	lhs, r1
 * 	mov	rhs, r0
 *	jsr	pc, imul
 * 	mov	r1, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	imul
imul:
	/ TODO
	rts	pc

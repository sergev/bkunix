/*
 * lshr - 32-bit ">>" routine.
 * Calls to lshr are generated automatically by the C compiler.
 *
 * 	mov	value_low, r0
 * 	mov	value_high, r1
 * 	mov	count, (sp)
 *	jsr	pc, lshr
 * 	mov	r0, result_low
 * 	mov	r1, result_high
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	lshr
lshr:
	/ TODO
	rts	pc

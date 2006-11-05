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
	/ TODO
	rts	pc

/*
 * C return sequence which
 * sets errno, returns -1.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.comm	_errno,2

	.globl	cerror
cerror:
	mov	r0,_errno
	mov	$-1,r0
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

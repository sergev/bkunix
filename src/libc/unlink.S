/*
 * C library --
 *	error = unlink(string);
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_unlink, cerror
_unlink:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_unlink
2:	.word	0

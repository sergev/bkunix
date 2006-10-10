/*
 * C library --
 *	f = dup(of)
 *
 * f == -1 for error
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_dup, cerror
_dup:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	SYS_dup
	bcc	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc

/*
 * C library --
 *	error = stat(string, statbuf);
 *
 * int statbuf[18] or
 * char statbuf[36]
 * as appropriate
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_stat, cerror
_stat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	mov	6(r5),3f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_stat
2:	.word	0
3:	.word	0

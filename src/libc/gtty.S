/*
 * C library --
 *	error = gtty(filep, ttyvec);
 *
 * filep is descriptor of open tty
 * ttyvec[0, 1, 2] correspond to args of gtty
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_gtty, cerror
_gtty:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_gtty
2:	.word	0

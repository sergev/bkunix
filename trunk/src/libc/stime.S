/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_stime, cerror
_stime:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	sys	SYS_stime
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

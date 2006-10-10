/*
 * C library --
 *	tvec = time(tvec);
 *
 * tvec[0], tvec[1] contain the time
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_time
_time:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_time
	mov	r2,-(sp)
	mov	4(r5),r2
	mov	r0,(r2)+
	mov	r1,(r2)+
	mov	(sp)+,r2
	mov	(sp)+,r5
	rts	pc

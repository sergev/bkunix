/*
 * C library --
 *	pid = wait(&status);
 *
 * pid == -1 if error
 * status indicates fate of process, if given
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_wait, cerror
_wait:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	SYS_wait
	bcc	1f
	jmp	cerror
1:
	mov	r1,*4(r5)	/ status return
	clr	r0
	mov	(sp)+,r5
	rts	pc

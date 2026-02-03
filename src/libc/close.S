/*
 * C library --
 *	error =  close(file);
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <syscall.h>

	.globl	_close, cerror
_close:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	SYS_close
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

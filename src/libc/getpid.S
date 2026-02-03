/*
 * getpid -- get process ID
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <syscall.h>

	.globl	_getpid
_getpid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_getpid
	mov	(sp)+,r5
	rts	pc

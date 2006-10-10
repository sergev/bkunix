/*
 * C library --
 *	execl(file, arg1, arg2, ... , 0);
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_execl, cerror
_execl:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	mov	r5,r0
	add	$6,r0
	mov	r0,3f
	sys	0
	.word	9f
	jmp	cerror

	.data
9:	sys	SYS_exec
2:	.word	0
3:	.word	0

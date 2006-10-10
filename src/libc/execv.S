/*
 * C library --
 *	execv(file, argv);
 *
 * where argv is a vector argv[0] ... argv[x], 0
 * last vector element must be 0
 *
 * The _exectrap flags is used by the debugger and causes
 * a trace trap on the first instruction of the executed instruction
 * to give a chance to set breakpoints.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.comm	__exectrap,2

	.globl	_execv, cerror

_execv:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	mov	6(r5),3f
	tst	__exectrap
	beq	1f
	mov	$0170020,-(sp)	/ t-bit
	mov	$1f,-(sp)
	rtt
1:
	sys	0
	.word	9f
	jmp	cerror

	.data
9:	sys	SYS_exec
2:	.word	0
3:	.word	0

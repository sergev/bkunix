/*
 * C library --
 *	exit(code)
 *
 * Code is return in r0 to system.
 */
#include <syscall.h>

	.globl	_exit
_exit:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	SYS_exit

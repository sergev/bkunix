/*
 * getpid -- get process ID
 */
#include <syscall.h>

	.globl	_getpid
_getpid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_getpid
	mov	(sp)+,r5
	rts	pc

/*
 * C library --
 *	uid = getuid();
 */
#include <syscall.h>

	.globl	_getuid
_getuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_getuid
	mov	(sp)+,r5
	rts	pc

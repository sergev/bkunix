#include <syscall.h>

	.globl	_sync
_sync:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_sync
	mov	(sp)+,r5
	rts	pc

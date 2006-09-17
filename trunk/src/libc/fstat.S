/*
 * C library --
 *	error = fstat(file, statbuf);
 *
 * int statbuf[17] or
 * char statbuf[34]
 * as appropriate
 */
#include <syscall.h>

	.globl	_fstat, cerror
_fstat:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_fstat
2:	.word	0

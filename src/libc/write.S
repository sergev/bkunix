/*
 * C library --
 *	nwritten = write(file, buffer, count);
 *
 * nwritten == -1 means error
 */
#include <syscall.h>

	.globl	_write, cerror
_write:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),2f
	mov	8(r5),3f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_write
2:	.word	0
3:	.word	0

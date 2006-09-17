/*
 * C library --
 *	error = mknod(string, mode, major.minor);
 */
#include <syscall.h>

	.globl	_mknod, cerror
_mknod:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	mov	6(r5),3f
	mov	8(r5),4f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_mknod
2:	.word	0
3:	.word	0
4:	.word	0

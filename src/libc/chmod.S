/*
 * C library --
 *	error = chmod(string, mode);
 */
#include <syscall.h>

	.globl	_chmod, cerror
_chmod:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	mov	6(r5),3f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_chmod
2:	.word	0
3:	.word	0

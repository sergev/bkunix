/*
 * C library --
 *	error = chdir(string);
 */
#include <syscall.h>

	.globl	_chdir, cerror
_chdir:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_chdir
2:	.word	0

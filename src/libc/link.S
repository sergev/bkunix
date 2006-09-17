/*
 * C library --
 *	error = link(old-file, new-file);
 */
#include <syscall.h>

	.globl	_link, cerror
_link:
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
9:	sys	SYS_link
2:	.word	0
3:	.word	0

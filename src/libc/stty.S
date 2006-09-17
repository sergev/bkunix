/*
 * C library --
 *	error = stty(filep, ttyvec);
 *
 * filep is descriptor of open tty
 * ttyvec[0, 1, 2] correspond to args of stty
 */
#include <syscall.h>

	.globl	_stty, cerror
_stty:
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
9:	sys	SYS_stty
2:	.word	0

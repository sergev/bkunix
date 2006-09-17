/*
 * C library --
 *	pid = fork();
 *
 * pid == 0 in child process;
 * pid == -1 means error return in child
 */
#include <syscall.h>

	.comm	_par_uid,2

	.globl	_fork, cerror
_fork:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_fork
		br 1f
	bcc	2f
	jmp	cerror
1:
	mov	r0,_par_uid
	clr	r0
2:
	mov	(sp)+,r5
	rts	pc

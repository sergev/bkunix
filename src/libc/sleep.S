/*
 * C library --
 *      sleep(sec)
 *
 * The process uses alarm(2) and pause(2) to
 * suspend execution for no less than sec seconds.
 */
#include <syscall.h>

	.globl	_sleep
_sleep:
 	mov	r5, -(sp)
 	mov	sp, r5
 	sys	SYS_signal
 	.word	14	/ SIGCLK (SIGALRM)
 	.word	1f
 	mov	4(r5), r0
 	sys	SYS_alarm
 	sys	SYS_pause
 	clr	r0
 	sys	SYS_alarm
 	mov	(sp)+, r5
 	rts	pc
1: 	rti

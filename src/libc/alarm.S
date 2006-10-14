/*
 * C library --
 *      alarm(sec) and pause()
 */
#include <syscall.h>

	.globl	_alarm, _pause
_alarm:
	mov	r5, -(sp)
	mov	sp, r5
	mov	4(r5), r0
	sys	SYS_alarm
	mov	(sp)+, r5
	rts	pc
_pause:
	mov	r5, -(sp)
	mov	sp, r5
	sys	SYS_pause
	mov	(sp)+, r5
	rts	pc

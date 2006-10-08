/*
 * C runtime startoff
 */
#include <syscall.h>

	.globl	_exit
	.globl	_main
start:
	mov	sp,r0
	mov	(r0),-(sp)
	tst	(r0)+
	mov	r0,2(sp)
#ifdef LOWSTACK
/ find the address of the word after the one containing
/ \0 of the last command line argument
	.globl	nd
	.data
nd:	.word	_end
	.text
	add	(sp),r0
	add	(sp),r0
	mov	-2(r0),r0
1:	tstb	(r0)+
	bne	1b
	mov	r0,nd
	asr	r0
	adc	nd
#endif
	jsr	pc,_main
	mov	r0,(sp)
	jsr	pc,*$_exit
	sys	SYS_exit

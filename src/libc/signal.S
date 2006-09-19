/*
 * C library --
 *	signal(n, 0);		-- default action on signal(n)
 *	signal(n, odd);		-- ignore signal(n)
 *	signal(n, label);	-- goto label on signal(n)
 *
 * returns old label, only one level.
 */
#include <syscall.h>

NSIG = 20
	.comm	__sigvect, NSIG*2

	.globl	_signal, cerror
_signal:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r1
	cmp	r1,$NSIG
	bhis	2f
	mov	6(r5),r0
	mov	r1,4f
	asl	r1
	mov	__sigvect(r1),-(sp)
	mov	r0,__sigvect(r1)
	mov	r0,5f
	beq	1f
	bit	$1,r0
	bne	1f
	asl	r1
	asl	r1
	add	$tvect,r1
	mov	r1,5f
1:
	sys	0
	.word	9f
	bcs	3f
	bit	$1,r0
	beq	1f
	mov	r0,(sp)
1:
	mov	(sp)+,r0
	mov	(sp)+,r5
	rts	pc
2:
	mov	$22,r0		/ EINVAL
	clr	-(sp)
3:
	tst	(sp)+
	jmp	cerror

tvect:
	mov r0,-(sp); mov __sigvect+[0*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[1*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[2*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[3*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[4*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[5*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[6*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[7*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[8*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[9*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[10*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[11*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[12*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[13*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[14*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[15*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[16*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[17*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[18*2],r0; br 1f;
	mov r0,-(sp); mov __sigvect+[19*2],r0; br 1f;
1:
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	jsr	pc,(r0)
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	rtt

	.data
9:	sys	SYS_signal
4:	.word	0
5:	.word	0

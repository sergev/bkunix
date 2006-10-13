/*
 * Machine language assist for LSI-11.
 *
 * Copyright 1975 Bell Telephone Laboratories Inc.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"

	.comm	_user, 2

#ifdef BGOPTION
	.globl	_swtch
#endif
	.globl	_trap

/-----------------------
	.globl	trap
trap:
	mfps	-4(sp)
	bit	$4,*$0177716
	beq	1f
	.globl	_stop
	inc	_stop
	tst	(sp)
	bpl	1f
	rti
1:
	jsr	r0,call1
	.word _trap
	/ no return
call1:
	tst	-(sp)
	clr	-(sp)
	mtps	(sp)+
	br	1f

/-----------------------
	.globl	call
call:
	mfps	-(sp)
1:
	mov	r1,-(sp)
	mov	sp,r1		/ get stack pointer at trap
	mov	r1,-(sp)
	add	$10,(sp)
	mov	4(sp),-(sp)
	bic	$0177740,(sp)	/ trap type
	cmp	10(sp),$ TOPSYS
	bhis	2f		/ trap from user
	cmp	10(sp),$ BOTSYS
	bhis	1f		/ trap from system
2:
	inc	_user
	mov	2(sp),r1	/ trap from user, get user stack
	mov	$ BOTUSR,sp
	mov	-(r1),-(sp)	/ copy user stack to system stack
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	jsr	pc,*(r0)+
#ifdef BGOPTION
	clr	-(sp)
	jsr	pc,*$_swtch
	tst	(sp)+
#endif
	mov	$0340,-(sp)
	mtps	(sp)+
	br	2f
1:
	clr	_user
	jsr	pc,*(r0)+
#ifdef BGOPTION
	clr	-(sp)
	jsr	pc,*$_swtch
	tst	(sp)+
#endif
2:
	tst	(sp)+
	mov	(sp)+,r1	/ new user stack pointer
	cmp	6(sp),$ TOPSYS
	bhis	2f		/ return from user trap
	cmp	6(sp),$ BOTSYS
	bhis	1f		/ return from system trap
2:
	sub	$10,r1		/ begin. of system stack to be copied to user stack
	mov	(sp)+,(r1)+	/ copy system stack to user stack
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	sub	$10,r1
	mov	r1,sp		/ switch to user stack
1:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rti

/-----------------------
	.globl	_idle
_idle:
	mfps	-(sp)
	clr	-(sp)
	mtps	(sp)+
	wait
	mtps	(sp)+
	tst	_stop
	beq	1f
	mov	$2,-(sp)
	clr	_user
	.globl	_signal
	jsr	pc,_signal
	tst	(sp)+
	clr	_stop
1:
	rts	pc

/-----------------------
	.globl	_savu
_savu:
	mov	$0340,r0
	mtps	r0
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
	clr	r0
	mtps	r0
	jmp	(r1)

/-----------------------
	.globl	_retu
_retu:
	mov	$0340,r0
	mtps	r0
	mov	(sp)+,r1
	mov	(sp),r0
	mov	(r0)+,sp
	mov	(r0)+,r5
	clr	r0
	mtps	r0
	jmp	(r1)

/-----------------------
	.globl	_spl0
_spl0:
	mfps	r0
	clr	r1
	mtps	r1
	rts	pc

/-----------------------
	.globl	_spl7
_spl7:
	mfps	r0
	mov	$0340,r1
	mtps	r1
	rts	pc

/-----------------------
	.globl _splx
_splx:
	mtps	2(sp)
	rts	pc

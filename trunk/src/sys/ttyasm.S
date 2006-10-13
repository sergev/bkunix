/*
 * Character list get/put
 *
 * Copyright 1975 Bell Telephone Laboratories Inc.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"

	.globl	_getc
	.globl	_cfreelist
_getc:
	mov	2(sp),r1
	mfps	-(sp)
	mov	r2,-(sp)
	mov	$0340,-(sp)
	mtps	(sp)+
	mov	2(r1),r2	/ first ptr
	beq	9f		/ empty
	movb	(r2)+,r0	/ character
	bic	$0xff00,r0
	mov	r2,2(r1)
	dec	(r1)+		/ count
	bne	1f
	clr	(r1)+
	clr	(r1)+		/ last block
	br	2f
1:
	bit	$7,r2
	bne	3f
	mov	-010(r2),(r1)	/ next block
	add	$2,(r1)
2:
	dec	r2
	bic	$7,r2
	mov	_cfreelist,(r2)
	mov	r2,_cfreelist
3:
	mov	(sp)+,r2
	mtps	(sp)+
	rts	pc
9:
	clr	4(r1)
	mov	$-1,r0
	mov	(sp)+,r2
	mtps	(sp)+
	rts	pc

/-----------------------
	.globl	_putc
_putc:
	mov	2(sp),r0
	mov	4(sp),r1
	mfps	-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	$0340,-(sp)
	mtps	(sp)+
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
	clr	(r2)+
	mov	r2,2(r1)	/ first ptr
	br	2f
1:
	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
	mov	r3,-010(r2)
	mov	r3,r2
	clr	(r2)+
2:
	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)		/ count
	clr	r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mtps	(sp)+
	rts	pc
9:
	mov	pc,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mtps	(sp)+
	rts	pc

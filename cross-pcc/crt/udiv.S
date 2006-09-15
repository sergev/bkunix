/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)udiv.s	2.3 (Berkeley) 1/28/87\0>
	.even
#endif LIBC_SCCS

/*
 * udiv(lhs::r0, rhs::r1)
 *	unsigned	lhs, rhs;
 *
 * urem(lhs::r0, rhs::r1)
 *	unsigned	lhs, rhs;
 *
 * Return unsigned div/rem and condition codes of result.
 */
#include "DEFS.h"

ASENTRY(udiv)
	cmp	r1,$1		/ if rhs > 1 && rhs < (2^16)/2 /*tricky*/
	ble	9f		/   return ((unsigned long)lhs)/rhs
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	(sp)+,r0
	rts	pc
9:
	bne	9f		/ else if rhs == 1
	tst	r0		/   return lhs
	rts	pc
9:
	cmp	r1,r0		/ else if lhs < rhs
	blos	9f		/   return 0
	clr	r0
	rts	pc
9:
	mov	$1,r0		/ else return 1
	rts	pc

ASENTRY(urem)
	cmp	r1,$1		/ if rhs > 1 && rhs < (2^16)/2 /*tricky*/
	ble	9f		/   return ((unsigned long)lhs)%rhs
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	(sp)+,r0
	mov	r1,r0		/ (div leaves "%" in r1)
	rts	pc
9:
	bne	9f		/ else if rhs == 1
	clr	r0		/   return 0
	rts	pc
9:
	cmp	r0,r1		/ else if lhs >= rhs
	blo	9f		/   return lhs - rhs
	sub	r1,r0
9:
	tst	r0		/ else return lhs
	rts	pc

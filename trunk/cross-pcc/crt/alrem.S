/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)alrem.s	2.4 (2.11BSD GTE) 12/26/92\0>
	.even
#endif LIBC_SCCS

/*
 * alrem(lhs, rhs)
 *	long	*lhs, rhs;
 *
 * 32-bit "/=" routine.  Calls to aldiv are generated automatically by the C
 * compiler.  See lrem for more detailed comments.
 */
#include "DEFS.h"

/*
 * Alrem for floating point hardware.  Check for divide by zero.  Don't want
 * floating divide trap in integer math.
 */
#define	one	040200

ASENTRY(alrem)
	tst	4(sp)		/ divide by zero check
	bne	1f
	tst	6(sp)
	bne	1f
	mov	2(sp),r1	/ return lhs
	mov	(r1)+,r0
	mov	(r1),r1
	rts	pc
1:
	setl
	mov	2(sp),r1
	movif	(r1),fr0	/ fr0 = *lhs
	movf	fr0,fr2		/ fr2 = *lhs
	movif	4(sp),fr3	/ fr3 = rhs
	divf	fr3,fr0		/ fr0 = *lhs/rhs
	modf	$one,fr0	/ fr1 = integer((*lhs/rhs) * 1.0)
	mulf	fr3,fr1		/ fr1 = integer(*lhs/rhs) * rhs
	subf	fr1,fr2		/ fr2 = *lhs - (integer(*lhs/rhs) * rhs)
	movfi	fr2,(r1)	/ *lhs = fr2
	mov	(r1)+,r0	/ and return result
	mov	(r1),r1
	seti
	rts	pc
#ifdef	never
/*
 * Alrem for fixed point hardware.
 */
.globl	lrem			/ 32-bit "%" routine

ASENTRY(alrem)
	mov	r2,-(sp)	/ need a register to point at the lhs
	mov	8.(sp),-(sp)	/ The fixed point remainder algorithm is long
	mov	8.(sp),-(sp)	/   enough that it just doesn't make sense
	mov	8.(sp),r2	/   to bother repeating it.  We just translate
	mov	2(r2),-(sp)	/   the call for lrem and let it do the work
	mov	(r2),-(sp)	/   and return its results (also stuffing it
	jsr	pc,lrem		/   into *lhs)
	add	8.,sp		/ clean up stack
	mov	r0,(r2)+	/ store high word,
	mov	r1,(r2)		/   and low
	mov	(sp)+,r2	/ restore r2
	rts	pc		/   and return
#endif

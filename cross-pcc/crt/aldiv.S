/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)aldiv.s	2.4 (2.11BSD GTE) 12/26/92\0>
	.even
#endif LIBC_SCCS

/*
 * aldiv(lhs, rhs)
 *	long	*lhs, rhs;
 *
 * 32-bit "/=" routine.  Calls to aldiv are generated automatically by the C
 * compiler.  See ldiv for more detailed comments.
 */
#include "DEFS.h"

/*
 * Aldiv for floating point hardware.  Check for divide by zero.  Don't want
 * floating divide trap in integer math.
 */
ASENTRY(aldiv)
	tst	4(sp)		/ divide by zero check
	bne	1f
	tst	6(sp)
	bne	1f
	mov	2(sp),r1	/ return lhs
	mov	(r1)+,r0	/
	mov	(r1),r1
	rts	pc
1:
	setl
	mov	2(sp),r1	/ r1 = lhs
	movif	(r1),fr0	/ fr0 = *lhs
	movif	4(sp),fr1	/ fr1 = rhs
	divf	fr1,fr0		/ fr0 /= rhs
	movfi	fr0,(r1)	/ *lhs = fr0
	mov	(r1)+,r0	/ and return result
	mov	(r1),r1
	seti
	rts	pc
#ifdef	never
/*
 * Aldiv for fixed point hardware.
 */
.globl	ldiv			/ 32-bit "/" routine

ASENTRY(aldiv)
	mov	r2,-(sp)	/ need a register to point at the lhs
	mov	8.(sp),-(sp)	/ The fixed point divide algorithm is long
	mov	8.(sp),-(sp)	/   enough that it just doesn't make sense
	mov	8.(sp),r0	/   to bother repeating it.  We just translate
	mov	2(r0),-(sp)	/   the call for ldiv and let it do the work
	mov	(r0),-(sp)	/   and return its results (also stuffing it
	jsr	pc,ldiv		/   into *lhs)
	add	8.,sp		/ clean up stack
	mov	r0,(r2)+	/ store high word,
	mov	r1,(r2)		/   and low
	mov	(sp)+,r2	/ restore r2
	rts	pc		/   and return
#endif

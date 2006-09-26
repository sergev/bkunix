/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * aldiv(lhs, rhs)
 *	long	*lhs, rhs;
 *
 * 32-bit "/=" routine.  Calls to aldiv are generated automatically by the C
 * compiler.  See ldiv for more detailed comments.
 */
#include "DEFS.h"

.globl	ldiv			/ 32-bit "/" routine

ASENTRY(aldiv)
	mov	r2,-(sp)	/ need a register to point at the lhs
	mov	8(sp),-(sp)	/ The fixed point divide algorithm is long
	mov	8(sp),-(sp)	/   enough that it just doesn't make sense
	mov	8(sp),r0	/   to bother repeating it.  We just translate
	mov	2(r0),-(sp)	/   the call for ldiv and let it do the work
	mov	(r0),-(sp)	/   and return its results (also stuffing it
	jsr	pc,ldiv		/   into *lhs)
	add	$8,sp		/ clean up stack
	mov	r0,(r2)+	/ store high word,
	mov	r1,(r2)		/   and low
	mov	(sp)+,r2	/ restore r2
	rts	pc		/   and return

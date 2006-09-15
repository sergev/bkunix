/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)lmul.s	2.3 (Berkeley) 1/28/87\0>
	.even
#endif LIBC_SCCS

/*
 * lmul(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "*" routine for fixed point hardware.  Also recommended for floating
 * hardware, for compatility & speed.  Credit to an unknown author who slipped
 * it under the door.  Calls to aldiv are generated automatically by the C
 * compiler.
 */
#include "DEFS.h"

ASENTRY(lmul)
	mov	r2,-(sp)	/ faster than csv and just 1 more word
	mov	r3,-(sp)
	mov	8.(sp),r2	/ r2 = loint(lhs)
	sxt	r1		/ r1 = sxt(loint(lhs)) - hiint(lhs)
	sub	6(sp),r1
	mov	12.(sp),r0	/ r0 = loint(rhs)
	sxt	r3		/ r3 = sxt(loint(rhs)) - hiint(rhs)
	sub	10.(sp),r3
	mul	r0,r1		/ MAGIC = loint(rhs) * hiint(lhs)'
	mul	r2,r3		/       + loint(lhs) * hiint(rhs)'
	add	r1,r3
	mul	r2,r0		/ prod (r0:r1) = loint(lhs)*loint(rhs)
	sub	r3,r0		/ hiint(prod) -= MAGIC
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

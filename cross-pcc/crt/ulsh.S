/*
 * Program: ulsh.s
 * Copyright 1993, GTE Government Systems
 * Author:  Steven M. Schultz
 *
 *  Version	Date		Modification
 *	0.0	02Feb91		1. Initial inspiration struck.
 *	1.0	05Jun93		2. Released into the Public Domain.
*/

#include "DEFS.h"

/*
 * All routines have both a C interface and an assembly interface.  Normally
 * the two are the same.  In the case of 'ulsh' the compiler has placed one
 * of the operands in r0 and r1 so the assembly interface differs from the
 * C interface.
*/

/*
 * u_long ulsh(lhs, count)
 *	u_long	lhs;
 *	short	count;
 *
 * 32-bit "<<" and ">>" routines.  Calls to ulsh are generated 
 * automatically by the C compiler.
 */

ASENTRY(ulsh)
	tst	2(sp)		/ shift count is on stack, long is in r0+r1
	bpl	1f
	ror	r0
	ror	r1
	inc	2(sp)
1:
	ashc	2(sp),r0
	rts	pc

ENTRY(ulsh)
	mov	2(sp),r0
	mov	4(sp),r1
	tst	6(sp)		/ positive count?
	bpl	1f		/ yes - br
	ror	r0		/ do the first shift
	ror	r1		/    the hard way
	inc	6(sp)		/ bump count towards zero
1:
	ashc	6(sp),r0	/ do the rest of the shifting
	rts	pc

/*
 * u_long ualsh(lhs, count)
 *	u_long	*lhs;
 *	short	count;
 *
 * 32-bit "<<=" and ">>=" routines.  Calls to ualsh are generated 
 * automatically by the C compiler.
 */
	.globl	ualsh
ualsh:
ENTRY(ualsh)
	mov	r2,-(sp)	/ save a register
	mov	4(sp),r2	/ *lhs
	mov	(r2)+,r0
	mov	(r2)+,r1
	tst	6(sp)		/ positive count?
	bpl	1f		/ yes - br
	ror	r0		/ do the first shift
	ror	r1		/    the hard way
	inc	6(sp)		/ bump count towards zero
1:
	ashc	6(sp),r0	/ do the rest of the shifting
	mov	r1,-(r2)
	mov	r0,-(r2)
	mov	(sp)+,r2
	rts	pc

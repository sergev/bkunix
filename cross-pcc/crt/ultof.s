/*
 * Program: ultof.s
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

#define	twogig	050000

#if	!defined(KERNEL)
/*
 * float ultof(lhs)
 *	u_long	lhs;
 *
 * unsigned 32-bit long to floating conversion.  Calls to ultof generated
 * automatically by the C compiler.  This routine is purposefully
 * not defined for the kernel since the kernel shouldn't (can't) do
 * FP arithmetic.
 */

	.globl ultof
ultof:
ENTRY(ultof)
	jsr	pc,l2f		/ 2(sp) -> fr0
	seti
	rts	pc

/*
 * Common sequences used more than once.  Moved here to save space at the
 * expense of a jsr+rts.  Both do a 'setl', the caller must do a 'seti'.
 * Not for the kernel until the kernel can do FP arithmetic.
*/

ASENTRY(l2f)
	setl
	tst	4(sp)
	bpl	1f
	bic	$100000,4(sp)
	movif	4(sp),fr0
	addf	$twogig,fr0
	rts	pc
1:
	movif	4(sp),fr0
	rts	pc

ASENTRY(l6f)
	setl
	tst	8.(sp)
	bpl	1f
	bic	$100000,8.(sp)
	movif	8.(sp),fr3
	addf	$twogig,fr3
	rts	pc
1:
	movif	8.(sp),fr3
	rts	pc
#endif KERNEL

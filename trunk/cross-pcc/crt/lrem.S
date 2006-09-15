/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)lrem.s	2.4 (GTE) 12/26/92\0>
	.even
#endif LIBC_SCCS

/*
 * lrem(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "%" routine.  Calls to lrem are generated automatically by the C
 * compiler.
 */
#include "DEFS.h"

#if !defined(KERNEL)
/*
 * Lrem for floating point hardware.  Check for divide by zero.  Don't want
 * floating point divide trap in integer math.
 */

#define	one	040200

ASENTRY(lrem)
	tst	6(sp)		/ divide by zero check
	bne	1f
	tst	8.(sp)
	bne	1f
	mov	2(sp),r0
	mov	4(sp),r1	/ return lhs
	rts	pc
1:
	setl
	movif	2(sp),fr0	/ fr0 = lhs
	movf	fr0,fr2		/ fr2 = lhs
	movif	6(sp),fr3	/ fr3 = rhs
	divf	fr3,fr0		/ fr0 = lhs/rhs
	modf	$one,fr0	/ fr0 = integer((lhs/rhs) * 1.0)
	mulf	fr3,fr1		/ fr0 = integer(lhs/rhs) * rhs
	subf	fr1,fr2		/ fr2 = lhs - (integer(*lhs/rhs) * rhs)
	movfi	fr2,-(sp)	/ (result)
	mov	(sp)+,r0
	mov	(sp)+,r1
	seti
	rts	pc
#else
/*
 * Lrem for fixed point hardware.
 */
#define	negl(high, low)	neg	high; \
			neg	low; \
			sbc	high	/ high -= (low != 0)

ASENTRY(lrem)
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	14.(sp),r3	/ r3 = loint(rhs)
	sxt	r4		/ r4 = sign(rhs)
	bpl	1f		/ if (int)loint(rhs) < 0
	neg	r3		/   r3 = asb(loint(rhs))
1:
	cmp	r4,12.(sp)	/ hiint(rhs) all sign bits?
	bne	hardlrem	/   no, rhs >= 2^15

	mov	10.(sp),r2	/ r2 = loint(lhs)
	mov	8.(sp),r1	/ r1 = hiint(lhs)
	bge	2f		/ if lhs < 0
	negl(r1, r2)		/   r1:r2 = abs(lhs)
2:
	/*
	 * At this point we know what the sign of the result is going to be
	 * (r4), abs(rhs) < 2^15, we have the absolute value of rhs in
	 * r3 as a single word integer and the absolute value of lhs in
	 * r1 (hiint) and r2 (loint).  References to hiint(rhs), loint(lhs)
	 * and hiint(lhs) in the following comments actually refer to the
	 * absolute value of rhs and lhs.
	 *
	 * We perform a long remainder via:
	 *	tmp        = (hiint(lhs) % loint(rhs))<<16 | loint(lhs)
	 *	loint(rem) = tmp % loint(rhs)
	 */
	clr	r0
	div	r3,r0		/ r1 = hiint(lhs) % loint(rhs)
	mov	r1,r4		/ stash hiint(tmp)
	mov	r1,r0		/ tmp=(hiint(lhs)%loint(rhs))<<16 | loint(lhs)
	mov	r2,r1		/ (r0:r1 = tmp)
	div	r3,r0		/ r1 = tmp % loint(rhs)
	bvc	3f		/ done if tmp/loint(rhs) < 2^15
	/*
	 * Our second division overflowed leaving undefined values in
	 * registers.  This can only happen when:
	 *	tmp/loint(rhs) >= 2^15
	 *	tmp >= 2^15 * loint(rhs)
	 *	tmp >= 2^16 * (loint(rhs)/2)
	 *
	 * If we subtract 2^16 * loint(rhs) from both sides however, we get:
	 *	tmp - (2^16 * loint(rhs)) >= -(2^16 * (loint(rhs)/2))
	 *
	 * and then divide both sides by loint(rhs):
	 *	tmp/loint(rhs) - 2^16 >= -(2^15)
	 *
	 * which is a division that won't generate an overflow.  Finally:
	 *	tmp = quo*loint(rhs) + rem
	 *	tmp - (2^16 * loint(rhs)) = (quo - 2^16) * loint(rhs) + rem
	 *
	 * Since we're now dividing a negative number and since the div
	 * instruction always generates a remainder the same sign as the
	 * dividend, if we get a non-zero remainder, we'll actually get:
	 *	(quo+1 - 2^16) * loint(rhs) + rem-loint(rhs)
	 *
	 * which means we'll have to adjust the remainder returned by
	 * adding loint(rhs) ...
	 */
	mov	r4,r0		/ reload r0:r1 with tmp (regs may be
	mov	r2,r1		/   clobbered by failed div)
	sub	r3,r0		/ r0:r1 -= 2^16 * loint(rhs)
	div	r3,r0
	tst	r1		/ if no remainder (0), bop out immediately,
	beq	4f		/   otherwise add loint(rhs)
	add	r3,r1
3:
	tst	8.(sp)		/ if lhs < 0  (result always sign of lhs)
	bpl	4f		/   rem = -rem
	neg	r1
4:
	sxt	r0		/ sign extend remainder
ret:
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/*
 * The divisor (rhs) is known to be >= 2^15 so we perform a bit shift
 * algorithm as only 16 cycles are needed:
 *	long
 *	hardlrem(lhs, rhs)
 *		long	lhs, rhs;
 *	{
 *		long		hi_sreg, lo_sreg;
 * 		unsigned int	cnt;
 *
 *		if (lhs < 0)
 *			lhs = -lhs;
 *		if (rhs < 0)
 *			rhs = -rhs;
 *		hi_sreg = hiint(lhs);
 *		lo_sreg = loint(lhs)<<16;
 *		for (cnt = 16; cnt; cnt--) {
 *			qshiftl(&hi_sreg, &lo_sreg);
 *			if (hi_sreg >= rhs)
 *				hi_sreg -= rhs;
 *		}
 *		return((long)((lhs < 0) ? -hi_sreg : hi_sreg));
 *	}
 * The assembly version of the above algorithm uses r0, r1 and r2 to implement
 * hi_sreg and lo_sreg by putting lhs into r0:r1 and zeroing r2 thereby
 * creating a three word register r2:r0:r1 with hi_sreg = r0:r1 and lo_sreg =
 * r1:r2 ...
 */
hardlrem:
	mov	10.(sp),r2	/ r2 = loint(lhs)
	mov	8.(sp),r1	/ r1 = hiint(lhs)
	bpl	1f		/ if lhs < 0
	negl(r1, r2)		/   r1:r2 = abs(lhs)
1:
	mov	12.(sp),r3	/ r3 = hiint(rhs)
	bge	2f		/ if rhs < 0
	negl(r3, 14.(sp))	/   rhs = -rhs (r3:loint(rhs))
2:
	clr	r0		/ clear top of shift register
	mov	$16.,r4		/ loop 16 times
3:
	clc			/ shift combined shift register and quotient
	rol	r2		/   left one place
	rol	r1
	rol	r0
	cmp	r3,r0		/ How do r0:r1 (hi_sreg) and rhs compare?
	bgt	4f
	blt	5f
	cmp	14.(sp),r1
	blos	5f
4:
	sob	r4,3b		/ r0:r1 (hi_sreg) < rhs:
	br	6f		/   just loop
5:
	sub	14.(sp),r1	/ r0:r1 (hi_sreg) >= rhs
	sbc	r0		/   subtract rhs from r0:r1 (hi_sreg)
	sub	r3,r0
	sob	r4,3b		/   and loop
6:
	tst	8(sp)		/ if lhs >= 0
	bge	ret		/   return immediately
	negl(r0, r1)		/ else negate answer before returning
	br	ret
#endif

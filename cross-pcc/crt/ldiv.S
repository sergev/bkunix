/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	<@(#)ldiv.s	2.4 (GTE) 12/26/92\0>
	.even
#endif LIBC_SCCS

/*
 * ldiv(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "/" routine.  Calls to ldiv are generated automatically by the C
 * compiler.
 */
#include "DEFS.h"

#if !defined(KERNEL)
/*
 * Ldiv for floating point hardware.  Check for divide by zero.  Don't want
 * floating divide trap in integer math.
 */
ASENTRY(ldiv)
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
	movif	6(sp),fr1	/ fr1 = rhs
	divf	fr1,fr0		/ fr0 /= rhs
	movfi	fr0,-(sp)
	mov	(sp)+,r0	/ return result
	mov	(sp)+,r1
	seti
	rts	pc
#else
/*
 * Ldiv for fixed point hardware.
 */
#define	negl(high, low)	neg	high; \
			neg	low; \
			sbc	high	/ high -= (low != 0)

ASENTRY(ldiv)
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	14.(sp),r3	/ r3 = loint(rhs)
	sxt	r4		/ r4 = sign(rhs)
	bpl	1f		/ if (int)loint(rhs) < 0
	neg	r3		/   r3 = asb(loint(rhs))
1:
	cmp	r4,12.(sp)	/ hiint(rhs) all sign bits?
	bne	hardldiv	/   no, rhs >= 2^15

	mov	10.(sp),r2	/ r2 = loint(lhs)
	mov	8.(sp),r1	/ r1 = hiint(lhs)
	bge	2f		/ if lhs < 0
	negl(r1, r2)		/   r1:r2 = abs(lhs)
	com	r4		/   invert sign of result
2:
	/*
	 * At this point we know what the sign of the result is going to be
	 * (r4), abs(rhs) < 2^15, we have the absolute value of rhs in
	 * r3 as a single word integer and the absolute value of lhs in
	 * r1 (hiint) and r2 (loint).  References to hiint(rhs), loint(lhs)
	 * and hiint(lhs) in the following comments actually refer to the
	 * absolute value of rhs and lhs.
	 *
	 * We perform a long division the same way you learned in school:
	 *	hiint(quo) = hiint(lhs) / loint(rhs)
	 *	tmp        = (hiint(lhs) % loint(rhs))<<16 | loint(lhs)
	 *	loint(quo) = tmp / loint(rhs)
	 */
	mov	r4,-(sp)	/ save sign of result
	clr	r0		/ r0 = hiint(lhs) / loint(rhs)
	div	r3,r0		/ r1 = hiint(lhs) % loint(rhs)
	mov	r0,r4		/ save high quotient
	mov	r1,-(sp)	/ stash hiint(tmp)
	mov	r1,r0		/ tmp=(hiint(lhs)%loint(rhs))<<16 | loint(lhs)
	mov	r2,r1		/ (r0:r1 = tmp)
	div	r3,r0		/ r0 = tmp / loint(rhs)
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
	 * which is fine since all we're going for is a 16 bit quotient so the
	 * subtraction of 2^16 won't have any effect.  However, since we're
	 * now dividing a negative number and since the div instruction
	 * always generates a remainder the same sign as the dividend, if we
	 * get a non-zero remainder, we'll actually get:
	 *	(quo+1 - 2^16) * loint(rhs) + rem-loint(rhs)
	 *
	 * which means we'll have to adjust the quotient returned by
	 * subtracting one ...
	 */
	mov	(sp),r0		/ reload r0:r1 with tmp (regs may be
	mov	r2,r1		/   clobbered by failed div)
	sub	r3,r0		/ r0:r1 -= 2^16 * loint(rhs)
	div	r3,r0
	tst	r1		/ if (negative) remainder, subtract one from
	sxt	r1		/   quotient
	add	r1,r0		/ cannot overflow!
3:
	tst	(sp)+		/ pop hiint(tmp) off stack
	mov	r0,r1		/ r1 (loint(quo)) = tmp / loint(rhs)
	mov	r4,r0		/ r0 (hiint(quo)) = hiint(lhs) / loint(rhs)
	tst	(sp)+
	bpl	ret
negret:				/ if result should be negative
	negl(r0, r1)		/   quo = -quo
ret:
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/*
 * The divisor (rhs) is known to be >= 2^15 so we perform a bit shift
 * algorithm as only 16 cycles are needed:
 *	long
 *	hardldiv(lhs, rhs)
 *		long	lhs, rhs;
 *	{
 *		int		flag;
 *		long		hi_sreg, lo_sreg;
 * 		unsigned int	quo, cnt;
 *
 *		flag = 0;
 *		if (lhs < 0) {
 *			lhs = -lhs;
 *			flag = !flag;
 *		}
 *		if (rhs < 0) {
 *			rhs = -rhs;
 *			flag = !flag;
 *		}
 *		hi_sreg = hiint(lhs);
 *		lo_sreg = loint(lhs)<<16;
 *		quo = 0;
 *		for (cnt = 16; cnt; cnt--) {
 *			quo <<= 1;
 *			qshiftl(&hi_sreg, &lo_sreg);
 *			if (hi_sreg >= rhs) {
 *				hi_sreg -= rhs;
 *				quo |= 1;
 *			}
 *		}
 *		return((long)(flag ? -quo : quo));
 *	}
 * The assembly version of the above algorithm uses r2, r0 and r1 to implement
 * hi_sreg, lo_sreg and quo by putting lhs into r0:r1 and zeroing r2 thereby
 * creating a three word register r2:r0:r1 with hi_sreg = r2:r0, lo_sreg =
 * r0:r1, and quo = r1 (using the unused bits in r1 as they become available
 * after the shift in the loop) ...
 */
hardldiv:
	mov	10.(sp),r1	/ r1 = loint(lhs)
	mov	8.(sp),r0	/ r0 = hiint(lhs)
	sxt	-(sp)		/ flag = sign(lhs)
	bpl	1f		/ if lhs < 0
	negl(r0, r1)		/   r0:r1 = abs(lhs)
1:
	mov	14.(sp),r3	/ r3 = hiint(rhs)
	bge	2f		/ if rhs < 0
	negl(r3, 16.(sp))	/   rhs = -rhs (r3:loint(rhs))
	com	(sp)		/   flag = !flag
2:
	clr	r2		/ clear top of shift register
	mov	$16.,r4		/ loop 16 times
3:
	clc			/ shift combined shift register and quotient
	rol	r1		/   left one place
	rol	r0
	rol	r2
	cmp	r3,r2		/ How do r2:r0 (hi_sreg) and rhs compare?
	bgt	4f
	blt	5f
	cmp	16.(sp),r0
	blos	5f
4:
	sob	r4,3b		/ r2:r0 (hi_sreg) < rhs:
	br	6f		/   just loop
5:
	sub	16.(sp),r0	/ r2:r0 (hi_sreg) >= rhs
	sbc	r2		/   subtract rhs from r2:r0 (hi_sreg)
	sub	r3,r2
	inc	r1		/   set bit in quotient
	sob	r4,3b		/   and loop
6:
	clr	r0		/ clear upper word of quotient
	tst	(sp)+		/ test negative flag
	bge	ret		/   and head off to the appropriate return
	br	negret
#endif

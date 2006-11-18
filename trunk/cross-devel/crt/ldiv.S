/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * ldiv(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "/" routine.  Calls to ldiv are generated automatically by the C
 * compiler.
 *
 * The divisor (rhs) is known to be >= 2^15 so we perform a bit shift
 * algorithm as only 16 cycles are needed:
 *	long
 *	hardldiv(lhs, rhs)
 *		long	lhs, rhs;
 *	{
 *		int		flag;
 *		long		hi_sreg, lo_sreg, quo;
 * 		unsigned int	cnt;
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
 * The assembly version of the above algorithm uses r2, r3, r0 and r1 to implement
 * hi_sreg, lo_sreg and quo by putting lhs into r0:r1 and zeroing r2:r3 thereby
 * creating a four word register r2:r3:r0:r1 with hi_sreg = r2:r3, lo_sreg =
 * r0:r1, and quo = r0:r1 (using the unused bits in r1 as they become available
 * after the shift in the loop) ...
 */
#define	negl(high, low)	neg	high; \
			neg	low; \
			sbc	high	/ high -= (low != 0)

	.globl	ldiv
ldiv:
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	10(sp),r1	/ r1 = loint(lhs)
	mov	8(sp),r0	/ r0 = hiint(lhs)
	sxt	-(sp)		/ flag = sign(lhs)
	bpl	1f		/ if lhs < 0
	negl(r0, r1)		/   r0:r1 = abs(lhs)
1:
	tst	14(sp)		/ hiint(rhs)
	bge	2f		/ if rhs < 0
	negl(14(sp), 16(sp))	/   rhs = -rhs
	com	(sp)		/   flag = !flag
2:
	clr	r2		/ clear top of shift register
	clr	r3
	mov	$32,r4		/ loop 16 times
3:
	clc			/ shift combined shift register and quotient
	rol	r1		/   left one place
	rol	r0
	rol	r3
	rol	r2
	cmp	14(sp),r2	/ How do r2:r3 (hi_sreg) and rhs compare?
	bgt	4f
	blt	5f
	cmp	16(sp),r3
	blos	5f
4:
	sob	r4,3b		/ r2:r3 (hi_sreg) < rhs:
	br	6f		/   just loop
5:
	sub	16(sp),r3	/ r2:r3 (hi_sreg) >= rhs
	sbc	r2		/   subtract rhs from r2:r3 (hi_sreg)
	sub	14(sp),r2
	inc	r1		/   set bit in quotient
	sob	r4,3b		/   and loop
6:
	tst	(sp)+		/ test negative flag
	bge	ret		/ if result should be negative
	negl(r0, r1)		/   quo = -quo
ret:
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

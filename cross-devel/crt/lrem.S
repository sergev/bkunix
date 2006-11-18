/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * lrem(lhs, rhs)
 *	long	lhs, rhs;
 *
 * 32-bit "%" routine.  Calls to lrem are generated automatically by the C
 * compiler.
 *
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
#define	negl(high, low)	neg	high; \
			neg	low; \
			sbc	high	/ high -= (low != 0)

	.globl	lrem
lrem:
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	10(sp),r1	/ r1 = loint(lhs)
	mov	8(sp),r0	/ r0 = hiint(lhs)
	bpl	1f		/ if lhs < 0
	negl(r0, r1)		/   r0:r1 = abs(lhs)
1:
	tst	12(sp)		/ hiint(rhs)
	bge	2f		/ if rhs < 0
	negl(12(sp), 14(sp))	/   rhs = -rhs
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
	cmp	12(sp),r2	/ How do r2:r3 (hi_sreg) and rhs compare?
	bgt	4f
	blt	5f
	cmp	14(sp),r3
	blos	5f
4:
	sob	r4,3b		/ r2:r3 (hi_sreg) < rhs:
	br	6f		/   just loop
5:
	sub	14(sp),r3	/ r2:r3 (hi_sreg) >= rhs
	sbc	r2		/   subtract rhs from r2:r3 (hi_sreg)
	sub	12(sp),r2
	sob	r4,3b		/   and loop
6:
	mov	r2,r0
	mov	r3,r1
	tst	8(sp)		/ if lhs >= 0
	bge	ret		/   return immediately
	negl(r0, r1)		/ else negate answer before returning
ret:
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

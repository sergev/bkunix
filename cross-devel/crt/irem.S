/*
 * irem - 16-bit "*" routine for fixed point hardware.
 * Calls to irem are generated automatically by the C compiler.
 * Usage:
 * 	mov	lhs, r1
 * 	mov	rhs, r0
 *	jsr	pc, irem
 * 	mov	r1, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 *
 * Perform a bit shift algorithm as only 16 cycles are needed:
 *	int
 *	lrem(lhs, rhs)
 *	{
 *		int		hi_sreg, lo_sreg;
 * 		unsigned int	cnt;
 *
 *		if (lhs < 0)
 *			lhs = -lhs;
 *		if (rhs < 0)
 *			rhs = -rhs;
 *		hi_sreg = 0;
 *		lo_sreg = lhs;
 *		for (cnt = 16; cnt; cnt--) {
 *			shiftl(&hi_sreg, &lo_sreg);
 *			if (hi_sreg >= rhs)
 *				hi_sreg -= rhs;
 *		}
 *		return (lhs < 0) ? -hi_sreg : hi_sreg;
 *	}
 * The assembly version of the above algorithm uses r1 and r2 to implement
 * hi_sreg and lo_sreg by putting lhs into r1 and zeroing r2 thereby
 * creating a two word register r2:r1 with hi_sreg = r2 and lo_sreg = r1 ...
 */
	.globl	irem
irem:
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r4,-(sp)
	mov	r1,-(sp)	/ save lhs
	bpl	1f		/ if lhs < 0
	neg	r1		/   r1 = abs(lhs)
1:
	tst	r0		/ r0 = rhs
	bge	2f		/ if rhs < 0
	neg	r0		/   rhs = -rhs
2:
	clr	r2		/ clear top of shift register
	mov	$16,r4		/ loop 16 times
3:
	clc			/ shift combined shift register and quotient
	rol	r1		/   left one place
	rol	r2
	cmp	r0,r2		/ How do r2:r1 (hi_sreg) and rhs compare?
	bgt	5f		/ r2:r1 (hi_sreg) >= rhs:
	sub	r0,r2		/   subtract rhs from r2:r1 (hi_sreg)
5:
	sob	r4,3b		/   and loop

	tst	(sp)+		/ test negative flag
	bge	6f		/ if lhs < 0
	neg	r2		/   quo = -quo
6:
	mov	r2,r1
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r2
	rts	pc

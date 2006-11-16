/*
 * idiv - 16-bit "*" routine for fixed point hardware.
 * Calls to idiv are generated automatically by the C compiler.
 * Usage:
 * 	mov	lhs, r1
 * 	mov	rhs, r0
 *	jsr	pc, idiv
 * 	mov	r1, result
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 *
 * Perform a bit shift algorithm as only 16 cycles are needed:
 *	int
 *	idiv(lhs, rhs)
 *	{
 *		int		flag;
 *		int		hi_sreg, lo_sreg;
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
 *		hi_sreg = 0;
 *		lo_sreg = lhs;
 *		quo = 0;
 *		for (cnt = 16; cnt; cnt--) {
 *			quo <<= 1;
 *			shiftl(&hi_sreg, &lo_sreg);
 *			if (hi_sreg >= rhs) {
 *				hi_sreg -= rhs;
 *				quo |= 1;
 *			}
 *		}
 *		return flag ? -quo : quo;
 *	}
 * The assembly version of the above algorithm uses r2 and r1 to implement
 * hi_sreg, lo_sreg and quo by putting lhs into r1 and zeroing r2 thereby
 * creating a two word register r2:r1 with hi_sreg = r2, lo_sreg = r1,
 * and quo = r1 (using the unused bits in r1 as they become available
 * after the shift in the loop) ...
 */
	.globl	idiv
idiv:
	mov	r2,-(sp)	/ faster than csv/cret ...
	mov	r4,-(sp)
	tst	r1		/ r1 = lhs
	sxt	-(sp)		/ flag = sign(lhs)
	bpl	1f		/ if lhs < 0
	neg	r1		/   r1 = abs(lhs)
1:
	tst	r0		/ r0 = rhs
	bge	2f		/ if rhs < 0
	neg	r0		/   rhs = -rhs
	com	(sp)		/   flag = !flag
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
	inc	r1		/   set bit in quotient
5:
	sob	r4,3b		/   and loop

	tst	(sp)+		/ test negative flag
	bge	6f		/ if result should be negative
	neg	r1		/   quo = -quo
6:
	mov	r1,r0
	mov	(sp)+,r4	/ restore registers
	mov	(sp)+,r2
	rts	pc

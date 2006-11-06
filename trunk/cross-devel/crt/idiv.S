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
 */
	.globl	idiv
idiv:
	/ TODO
	rts	pc

#if 0
/ this routine performs the division function
/   the algorithm used is taken directly
/   from the DEC microcode flow in order
/   to make all condition codes and results
/   agree in all cases.
/
	jsr	pc,getsrc
	mov	r5,-(sp)
	mov	r1,r5
/ is divisor 0?
	tst	r2
	bne	div5
/ if so, quit
	br	div12
div5:
/ is dividend negative?
	tst	r0
	bge	div16
/ yes
	neg	r0
	clr	r1
	sub	r5,r1
	sbc	r0
	bpl	div16
div12:
/ error exit
	mov	(sp)+,r5
	mov	$17,r2
	jmp	setcc
div16:
/ first step of algorithm
	mov	r2,r3
	neg	r3
	clc
	rol	r1
	rol	r0
	tst	r2
	bmi	1f
	add	r3,r0
	br	2f
1:
	add	r2,r0
2:
	adc	r1
/ test for overflow condition
	bit	$1,r1
	beq	1f
	tst	r0
	jne	div12
	clr	r5
	tst	r2
	bge	3f
	inc	r5
3:
	bit	$10,*r4
	beq	3f
	inc	r5
3:
	ror	r5
	jcc	div12
1:
/ if no overflow, interate algorithm 15 times
	mov	$17,-(sp)
div19:
	clc
	rol	r1
	rol	r0
	clr	r5
	tst	r2
	bpl	1f
	inc	r5
1:
	bit	$2,r1
	beq	1f
	inc	r5
1:
	ror	r5
	bcc	1f
	add	r3,r0
	br	2f
1:
	add	r2,r0
2:
	adc	r1
	dec	(sp)
	bne	div19
/ branch to appropriate cleanup and condition code routines
	cmp	r2,(sp)+
	bgt	1f
	bit	$1,r1
	beq	div31
	bne	div33
1:
	bit	$1,r1
	bne	div23
div27:
	add	r2,r0
div23:
	bit	$10,*r4
	bne	div29
div26:
	jmp	div42
div31:
	sub	r2,r0
div33:
	bit	$10,*r4
	beq	div37
	neg	r0
	jmp	div26
div29:
	neg	r0
div37:
	mov	r1,r3
	neg	r1
	com	r3
	jmi	div26
div42:
/ exit procedure
	mov	(sp)+,r5
	mov	r0,r3
	mov	r1,r0
	mov	r3,r1
	mov	$1,r3
	mov	$-17,r2
	jmp	strreg
#endif

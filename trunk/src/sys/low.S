/*
 * Low core startup code.
 *
 * Copyright 1975 Bell Telephone Laboratories Inc.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"

	.globl	start		/ system start (mch.s)
	.globl	call		/ interrupt handler (mch.s)
	.globl	trap		/ trap handler (mch.s)

#define IM	0340		/* mask interrupts */

/-----------------------
/ Address 0: system starts here.
/
init:	br	1f		/ not enough place here
	.word	4

/-----------------------
/ Trap vectors
/
	.word	trap, IM+0	/ bus error
	.word	trap, IM+1	/ illegal instruction
	.word	trap, IM+2	/ bpt-trace trap
	.word	trap, IM+3	/ iot trap
	.word	trap, IM+4	/ power fail
	.word	trap, IM+5	/ emulator trap
	.word	trap, IM+6	/ system entry

/-----------------------
	.=	init+040
1:	jmp	start		/ jump to code in mch.s

/-----------------------
#ifdef KL
	.=	init+060	/ KL/DL-11 console interrupt handlers
	.word	klin, IM
	.word	klou, IM
#endif

/-----------------------
#ifdef CLOCK
	.=	init+0100	/ clock interrupt handlers
	.word	kwlp, IM
	.word	kwlp, IM
#endif

/-----------------------
#ifdef PER
	.=	init+0124	/ PER floppy interrupt handler
	.word	fdintr, IM
#endif

/-----------------------
#ifdef KL
	.globl	_klrint
klin:	jsr	r0,call
	.word	_klrint

	.globl	_klxint
klou:	jsr	r0,call
	.word	_klxint
#endif

/-----------------------
#ifdef CLOCK
	.globl	_clock
kwlp:	jsr	r0,call
	.word	_clock
#endif

/-----------------------
	.globl	_fdintr
fdintr:	jsr	r0,call
	.word	_fdintr

/-----------------------
#ifdef AED
	.=	init+0170	/ AED 6200lp floppy interrupt handler
	.word	fdintr, IM
#endif

/-----------------------
#ifdef PER
	.=	init+0174	/ PER floppy interrupt handler
	.word	fdintr, IM
#endif

/-----------------------
#ifdef SYK
	.=	init+0174	/ Sykes floppy interrupt handler
	.word	fdintr, IM
#endif

/-----------------------
#ifdef TVT
	.=	init+0200	/ TV terminal interrupt handlers
	.word	klin, IM
#endif

/-----------------------
#ifdef RF
	.=	init+0204	/ RF floppy interrupt handler
	.word	fdintr, IM
#endif

/-----------------------
#ifdef DEC
	.=	init+0264	/ DEC floppy interrupt handler
	.word	fdintr, IM
#endif

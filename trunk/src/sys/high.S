/*
 * High core startup code for BK-0011.
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
init:	clr	r0
	mov	go, (r0)+	/ restart if jumped to 0
	mov	$init, (r0)+
	mov	$trap, r1
	mov	$ IM, r2
1:	mov	r1, (r0)+
	mov	r2, (r0)+
	inc	r2
	cmp	$ IM+7, r2
	bne	1b
/-----------------------
#ifdef KL
	mov	$060, r0	/ KL/DL-11 console interrupt handlers
	mov	$klin, (r0)+
	mov	$ IM, (r0)+
#ifdef DEC
	mov	$klou, (r0)+
	mov	$ IM, (r0)+
#endif
#endif

/-----------------------
#ifdef CLOCK
	mov	$0100, r0	/ clock interrupt handlers
	mov	$kwlp, (r0)+
	mov	$ IM, (r0)+
	mov	$kwlp, (r0)+
	mov	$ IM, (r0)+
#endif

/-----------------------
#ifdef DEC
	mov	$0264, r0	/ DEC floppy interrupt handler
	mov	$fdintr, (r0)+
	mov	$ IM, (r0)+
#endif

#ifdef BK
	mov	$0274, r0
	mov	$klin, (r0)+
	mov	$ IM, (r0)+
#endif
go:	jmp	*$start

/-----------------------
#ifdef KL
	.globl	_klrint
klin:	jsr	r0,call
	.word	_klrint
#ifdef DEC
	.globl	_klxint
klou:	jsr	r0,call
	.word	_klxint
#endif
#endif

/-----------------------
#ifdef CLOCK
	.globl	_clock
kwlp:	jsr	r0,call
	.word	_clock
#endif

/-----------------------
#ifdef DEC
	.globl	_fdintr
fdintr:	jsr	r0,call
	.word	_fdintr
#endif

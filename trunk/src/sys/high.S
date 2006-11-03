/*
 * High core startup code for BK-0011.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"

	.globl	call		/ interrupt handler (mch.s)
	.globl	trap		/ trap handler (mch.s)
	.globl	_edata
	.globl	_unixmain

usize	= 01000
_u	= BOTUSR - usize
	.globl	_u

#define IM	0340		/* mask interrupts */

	.globl	_bootdev
/-----------------------
/ System starts here.
/
init:
	mov	r0,*$_bootdev
	clr	r0
	mov	$0137, (r0)+	/ restart if jumped to 0
	mov	$init, (r0)+
	mov	$trap, r1
	mov	$ IM, r2
1:	mov	r1, (r0)+
	mov	r2, (r0)+
	inc	r2
	cmp	$ IM+7, r2
	bne	1b

	mov	$060, r0	/ console interrupt handlers
	mov	$klin, (r0)+
	mov	$ IM, (r0)+

	mov	$0160016, *$010 / EIS emulator
	mov	$0274, r0
	mov	$klin, (r0)+
	mov	$ IM, (r0)+
	reset

/ clear bss and user block
	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$ TOPSYS
	blo	1b

	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$ BOTUSR
	blo	1b

/ set up stack pointer
	mov	r0,sp

/ and pointer to system stack
	mov	$ TOPSYS-2,_u

/ set up previous mode and call main
/ on return, enter user mode at BOTUSR

	jsr	pc,_unixmain
	mov	$ BOTUSR+8192,sp	/ set stack at first 8kbytes of user space
	clr	-(sp)
	mov	$ BOTUSR,-(sp)
	rti

/-----------------------
	.globl	_ttyinput
klin:	jsr	r0,call
	.word	_ttyinput

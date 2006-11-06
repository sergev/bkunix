/*
 * Startup code for standalone utilities.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
TOPSYS = 0160000
	.globl	_edata
	.globl	_main
	.comm	_bootdev, 2

/-----------------------
/ System starts here.
/
init:
	mov	r0,*$_bootdev
	reset

	mov	$_edata,r0		/ clear bss and stack
1:	clr	(r0)+
	cmp	r0,$ TOPSYS
	blo	1b

	mov	r0,sp			/ set up stack pointer
	jsr	pc,_main
	jbr	init

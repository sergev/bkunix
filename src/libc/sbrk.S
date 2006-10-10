/*
 * old = sbrk(increment);
 *
 * sbrk gets increment more core, and returns a pointer
 * to the beginning of the new core area
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_sbrk, _end, cerror
_sbrk:
	mov	r5,-(sp)
	mov	sp,r5
	mov	nd,2f
	add	4(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	mov	nd,r0
	add	4(r5),nd
	mov	(sp)+,r5
	rts	pc

/*
 * brk(value)
 * as described in man2.
 * returns 0 for ok, -1 for error.
 */
	.globl	_brk
_brk:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),2f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	mov	4(r5),nd
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_break
2:	.word	0

#ifdef LOWSTACK
	.globl	nd
#else
nd:	.word	_end
#endif

/*
 * C library --
 * 	error = mount(dev, file, flag);
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_mount, cerror
_mount:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),2f
	mov	6(sp),3f
	mov	8(sp),4f
	sys	0
	.word	9f
	bcc	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

	.data
9:	sys	SYS_mount
2:	.word	0
3:	.word	0
4:	.word	0

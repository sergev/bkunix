/*
 * C library --
 *	uid = getuid();
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_getuid
_getuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	SYS_getuid
	mov	(sp)+,r5
	rts	pc

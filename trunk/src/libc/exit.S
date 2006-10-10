/*
 * C library --
 *	exit(code)
 *
 * Code is return in r0 to system.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <syscall.h>

	.globl	_exit
_exit:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	SYS_exit

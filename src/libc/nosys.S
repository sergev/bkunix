/*
 * Unavalable syscalls:
 *	37 = kill
 *	42 = pipe
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
	.globl	_kill, _pipe
_kill:
_pipe:
	mov	$-1, r0
	rts	pc

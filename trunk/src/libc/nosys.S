/*
 * Unavalable syscalls:
 *	37 = kill
 *	42 = pipe
 */
	.globl	_kill, _pipe
_kill:
_pipe:
	mov	$-1, r0
	rts	pc

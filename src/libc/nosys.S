/*
 * Unavalable syscalls:
 *	21 = mount
 *	22 = umount
 *	37 = kill
 *	42 = pipe
 */
	.globl	_mount, _umount, _kill, _pipe
_mount:
_umount:
_kill:
_pipe:
	mov	$-1, r0
	rts	pc

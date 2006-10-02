/*
 * Empty syscalls:
 *	0  = indir
 *	16 = chown
 *	23 = setuid
 *	26 = ptrace
 *	30 = smdate; inoperative
 *	34 = nice
 *	43 = times
 *	46 = setgid
 *	47 = getgid
 */
	.globl	_chown, _setuid, _nice, _times, _setgid, _getgid
_chown:
_setuid:
_nice:
_times:
_setgid:
_getgid:
	clr	r0
	rts	pc

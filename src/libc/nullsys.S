/*
 * Empty syscalls:
 *	16 = chown
 *	23 = setuid
 *	34 = nice
 *	43 = times
 *	46 = setgid
 *	47 = getgid
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
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

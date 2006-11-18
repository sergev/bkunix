/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

int
fputs(s, iop)
	register char *s;
	register FILE *iop;
{
	register r;
	register c;

	while ((c = *s++))
		r = putc(c, iop);
	return r;
}

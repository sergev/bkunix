/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
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

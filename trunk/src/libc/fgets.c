/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

char *
fgets(s, n, iop)
	char *s;
	register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
	while (--n>0 && (c = getc(iop))>=0) {
		*cs++ = c;
		if (c=='\n')
			break;
	}
	if (c<0 && cs==s)
		return NULL;
	*cs++ = '\0';
	return s;
}

/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

int
puts(s)
	register char *s;
{
	register c;

	while (c = *s++)
		putchar(c);
	return putchar('\n');
}

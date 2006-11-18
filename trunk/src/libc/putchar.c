/*
 * A subroutine version of the macro putchar
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#undef putchar

int
putchar(c)
	register c;
{
	return putc(c, stdout);
}

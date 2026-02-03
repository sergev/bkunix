/*
 * A subroutine version of the macro putchar
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

#undef putchar

int
putchar(c)
	register c;
{
	return putc(c, stdout);
}

/*
 * A subroutine version of the macro getchar.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#undef getchar

int
getchar()
{
	return getc(stdin);
}

/*
 * A subroutine version of the macro getchar.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

#undef getchar

int
getchar()
{
	return getc(stdin);
}

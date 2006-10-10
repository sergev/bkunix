/*
 * Calloc - allocate and clear memory block.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>

char *
calloc(num, size)
	register unsigned num, size;
{
	register char *p;

	size *= num;
	p = malloc(size);
	if (p)
		memset(p, 0, size);
	return p;
}

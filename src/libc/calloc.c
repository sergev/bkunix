/*
 * Calloc - allocate and clear memory block.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
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

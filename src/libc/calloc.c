#include <stdlib.h>

/*
 * Calloc - allocate and clear memory block
 */
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

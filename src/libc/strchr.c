/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 *
 * this routine is just "index" renamed.
 */
#include <string.h>

char *
strchr(sp, c)
	register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return 0;
}

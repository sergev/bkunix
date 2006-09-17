/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * This routine is just "rindex" renamed.
 */
#include <string.h>

char *
strrchr(sp, c)
	register char *sp, c;
{
	register char *r;

	r = 0;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}

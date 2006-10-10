/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * This routine is just "rindex" renamed.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
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

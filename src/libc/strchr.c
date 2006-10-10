/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 *
 * this routine is just "index" renamed.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
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

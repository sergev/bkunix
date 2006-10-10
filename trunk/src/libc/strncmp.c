/*
 * Compare strings (at most n bytes):  s1>s2: >0  s1==s2: 0  s1<s2: <0
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <string.h>

int
strncmp(s1, s2, n)
	register char *s1, *s2;
	register n;
{

	while (--n >= 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return(n<0 ? 0 : *s1 - *--s2);
}

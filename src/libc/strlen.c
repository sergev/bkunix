/*
 * Returns the number of
 * non-NULL bytes in string argument.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <string.h>

int
strlen(s)
	register char *s;
{
	register n;

	n = 0;
	while (*s++)
		n++;
	return(n);
}

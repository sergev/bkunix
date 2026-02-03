/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <string.h>

int
strcmp(s1, s2)
	register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(*s1 - *--s2);
}

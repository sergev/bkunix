/*
 * Convert ASCII string to integer.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>

int
atoi(ap)
	char *ap;
{
	register n, c;
	register char *p;
	int f;

	p = ap;
	n = 0;
	f = 0;
loop:
	while(*p == ' ' || *p == '	')
		p++;
	if(*p == '-') {
		f++;
		p++;
		goto loop;
	}
	while(*p >= '0' && *p <= '9')
		n = n*10 + *p++ - '0';
	if(f)
		n = -n;
	return(n);
}

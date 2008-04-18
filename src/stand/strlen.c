/*
 * Returns the number of
 * non-NULL bytes in string argument.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */

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

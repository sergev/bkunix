/*
 * Formatted output.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

int
vprintf (fmt, args)
	char *fmt;
	va_list args;
{
	int err;

	err = vfprintf (stdout, fmt, args);
	return err;
}

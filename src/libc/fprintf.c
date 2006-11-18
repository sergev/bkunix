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
fprintf (fdout, fmt)
	FILE *fdout;
	char *fmt;
{
	va_list	args;
	int err;

	va_start (args, fmt);
	err = vfprintf (fdout, fmt, args);
	va_end (args);
	return err;
}

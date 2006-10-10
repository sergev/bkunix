/*
 * Formatted output.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <stdarg.h>

int
printf (fmt)
	char *fmt;
{
	va_list	args;
	int err;

	va_start (args, fmt);
	err = vprintf (fmt, args);
	va_end (args);
	return err;
}

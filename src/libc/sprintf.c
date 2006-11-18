/*
 * Formatted output to string buffer.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdarg.h>
#include <stdio.h>

int
sprintf(str, fmt)
	char *str, *fmt;
{
	FILE _strbuf;
	va_list	args;
	int err;

	_strbuf._flag = _IOWRT + _IOSTRG;
	_strbuf._ptr = str;
	_strbuf._cnt = 32767;

	va_start (args, fmt);
	err = vfprintf (&_strbuf, fmt, args);
	va_end (args);

	putc('\0', &_strbuf);
	return err;
}

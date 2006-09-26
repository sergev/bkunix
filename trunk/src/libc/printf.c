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

#include <stdlib.h>

#define va_list		char*
#define va_start(p,a)	{ p = (char*) &a; p += sizeof(char*); }
#define va_end(p)
#define va_arg(p,t)	((t*) (p += sizeof(t)))[-1]

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

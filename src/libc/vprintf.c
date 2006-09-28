/*
 * Scaled down version of printf(3).
 * Based on FreeBSD sources, heavily rewritten.
 *
 * Copyright (C) 2000-2002 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this file and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING.txt" for more details.
 */
/*
 * Two additional formats:
 *
 * The format %b is supported to decode error registers.
 * Its usage is:
 *
 *	printf("reg=%b\n", regval, "<base><arg>*");
 *
 * where <base> is the output base expressed as a control character, e.g.
 * \10 gives octal; \20 gives hex.  Each arg is a sequence of characters,
 * the first of which gives the bit number to be inspected (origin 1), and
 * the next characters (up to a control character, i.e. a character <= 32),
 * give the name of the register.  Thus:
 *
 *	kvprintf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 *
 * would produce output:
 *
 *	reg=3<BITTWO,BITONE>
 *
 * The format %D -- Hexdump, takes a pointer. Sharp flag - use `:' as
 * a separator, instead of a space. For example:
 *
 *	("%6D", ptr)       -> XX XX XX XX XX XX
 *	("%#*D", len, ptr) -> XX:XX:XX:XX ...
 */
#include <stdlib.h>
#include <stdarg.h>

/* Max number conversion buffer length: a long in base 2, plus NUL byte. */
#define MAXNBUF	(sizeof(long) * 8 + 1)

static char
mkhex (ch)
	int ch;
{
	ch &= 15;
	if (ch > 9)
		return ch + 'a' - 10;
	return ch + '0';
}

/*
 * Put a NUL-terminated ASCII number (base <= 16) in a buffer in reverse
 * order; return an optional length and a pointer to the last character
 * written in the buffer (i.e., the first character of the string).
 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
static char *
ksprintn (nbuf, ul, base, width, lenp)
	char *nbuf;
	unsigned long ul;
	int base, width;
	unsigned char *lenp;
{
	char *p;
	int digit;

	p = nbuf;
	*p = 0;
	for (;;) {
		digit = ul % base;
		*++p = mkhex (digit);
		ul /= base;
		if (--width > 0)
			continue;
		if (! ul)
			break;
	}
	if (lenp)
		*lenp = p - nbuf;
	return (p);
}

static void
outchar (c)
	char c;
{
	write (1, &c, 1);
}

#if 0
static void printlong (a)
	long a;
{
	long b;

	if (a < 0) {
		outchar ('-');
		a = - a;
	}
	b = a / 10;
	if (b > 0) {
		printlong (b);
		a -= b * 10;
	}
	outchar ((int) a + '0');
}
#endif

int
vprintf (fmt, ap)
	char *fmt;
	va_list ap;
{
#define PUTC(c) { outchar(c); ++retval; }
	char nbuf [MAXNBUF], padding, *p, *q;
	unsigned char ch, base, lflag, ladjust, sharpflag, neg, sign, dot, size;
	unsigned char *up;
	int n, width, dwidth, retval, uppercase, extrazeros;
	unsigned long ul;

	if (! fmt)
		fmt = "(null)\n";

	retval = 0;
	for (;;) {
		ch = *fmt++;
		if (! ch)
			return retval;
		if (ch != '%') {
			PUTC (ch);
			continue;
		}
		padding = ' ';
		width = 0; extrazeros = 0;
		lflag = 0; ladjust = 0; sharpflag = 0; neg = 0;
		sign = 0; dot = 0; uppercase = 0; dwidth = -1;
reswitch:	ch = *fmt++;
		switch (ch) {
		case '.':
			dot = 1;
			padding = ' ';
			dwidth = 0;
			goto reswitch;

		case '#':
			sharpflag = 1;
			goto reswitch;

		case '+':
			sign = 1;
			goto reswitch;

		case '-':
			ladjust = 1;
			goto reswitch;

		case '%':
			PUTC (ch);
			break;

		case '*':
			if (! dot) {
				width = va_arg (ap, int);
				if (width < 0) {
					ladjust = !ladjust;
					width = -width;
				}
			} else {
				dwidth = va_arg (ap, int);
			}
			goto reswitch;

		case '0':
			if (! dot) {
				padding = '0';
				goto reswitch;
			}
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (n=0; ; ++fmt) {
				n = n * 10 + ch - '0';
				ch = *fmt;
				if (ch < '0' || ch > '9')
					break;
			}
			if (dot)
				dwidth = n;
			else
				width = n;
			goto reswitch;

		case 'b':
			ul = va_arg (ap, int);
			p = va_arg (ap, char*);
			q = ksprintn (nbuf, ul, *p++, -1, 0);
			while (*q)
				PUTC (*q--);

			if (! ul)
				break;
			size = 0;
			while (*p) {
				n = *p++;
				if ((char) (ul >> (n-1)) & 1) {
					PUTC (size ? ',' : '<');
					for (; (n = *p) > ' '; ++p)
						PUTC (n);
					size = 1;
				} else
					while (*p > ' ')
						++p;
			}
			if (size)
				PUTC ('>');
			break;

		case 'c':
			if (! ladjust && width > 0)
				while (width--)
					PUTC (' ');

			PUTC (va_arg (ap, int));

			if (ladjust && width > 0)
				while (width--)
					PUTC (' ');
			break;

		case 'D':
			up = va_arg (ap, unsigned char*);
			if (! width)
				width = 16;
			if (sharpflag)
				padding = ':';
			while (width--) {
				ch = *up++;
				PUTC (mkhex (ch >> 4));
				PUTC (mkhex (ch));
				if (width)
					PUTC (padding);
			}
			break;

		case 'd':
			ul = lflag ? va_arg (ap, long) : va_arg (ap, int);
			sign = 1;
			base = 10;
			goto number;

		case 'l':
			lflag = 1;
			goto reswitch;

		case 'o':
			ul = lflag ? va_arg (ap, unsigned long) :
				va_arg (ap, unsigned int);
			base = 8;
			goto nosign;

		case 'p':
			ul = (unsigned int) va_arg (ap, int);
			if (! ul) {
				p = "(nil)";
				goto string;
			}
			base = 16;
			sharpflag = (width == 0);
			goto nosign;

		case 'n':
		case 'r':
			ul = lflag ? va_arg (ap, unsigned long) :
				sign ? (unsigned long) va_arg (ap, int) :
				va_arg (ap, unsigned int);
			base = 10;
			goto number;

		case 's':
			p = va_arg (ap, char*);
string:
			if (! p)
				p = "(null)";

			if (! dot)
				n = strlen (p);
			else
				for (n=0; n<dwidth && p[n]; n++)
					continue;

			width -= n;

			if (! ladjust && width > 0)
				while (width--)
					PUTC (' ');
			while (n--)
				PUTC (*p++);
			if (ladjust && width > 0)
				while (width--)
					PUTC (' ');
			break;

		case 'u':
			ul = lflag ? va_arg (ap, unsigned long) :
				va_arg (ap, unsigned int);
			base = 10;
			goto nosign;

		case 'x':
		case 'X':
			ul = lflag ? va_arg (ap, unsigned long) :
				va_arg (ap, unsigned int);
			base = 16;
			uppercase = (ch == 'X');
			goto nosign;
		case 'z':
		case 'Z':
			ul = lflag ? va_arg (ap, unsigned long) :
				sign ? (unsigned long) va_arg (ap, int) :
				va_arg (ap, unsigned int);
			base = 16;
			uppercase = (ch == 'Z');
			goto number;

nosign:			sign = 0;
number:
/*PUTC('<');*/
/*printlong(ul);*/
/*PUTC('>');*/
			if (sign && (long) ul < 0L) {
				neg = 1;
				ul = -(long) ul;
/*PUTC('<');*/
/*printlong(ul);*/
/*PUTC('>');*/
			}
			if (dwidth > 0 && dwidth >= sizeof(nbuf)) {
				extrazeros = dwidth - sizeof(nbuf) + 1;
				dwidth = sizeof(nbuf) - 1;
			}
			p = ksprintn (nbuf, ul, base, dwidth, &size);
			if (sharpflag && ul != 0) {
				if (base == 8)
					size++;
				else if (base == 16)
					size += 2;
			}
			if (neg)
				size++;

			if (! ladjust && width && padding == ' ' &&
			    (width -= size) > 0)
				do {
					PUTC (' ');
				} while (--width > 0);

			if (neg)
				PUTC ('-');

			if (sharpflag && ul != 0) {
				if (base == 8) {
					PUTC ('0');
				} else if (base == 16) {
					PUTC ('0');
					PUTC (uppercase ? 'X' : 'x');
				}
			}

			if (extrazeros)
				do {
					PUTC ('0');
				} while (--extrazeros > 0);

			if (! ladjust && width && (width -= size) > 0)
				do {
					PUTC (padding);
				} while (--width > 0);

			for (; *p; --p) {
				if (uppercase && *p>='a' && *p<='z') {
					PUTC (*p + 'A' - 'a');
				} else {
					PUTC (*p);
				}
			}

			if (ladjust && width && (width -= size) > 0)
				do {
					PUTC (' ');
				} while (--width > 0);
			break;
		default:
			PUTC ('%');
			if (lflag)
				PUTC ('l');
			PUTC (ch);
			break;
		}
	}
}

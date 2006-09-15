
#ifdef STANDALONE
/*
 * Compile standalone binary file.
 * Used for testing the compiler.
 */
void debug_printf();
void debug_print_int();

void standalone ()
{
	debug_printf ("int=%x long=%lx\n", 0x1234, 0x5678abcdL);
	asm ("halt");
}
#endif

#ifdef DEBUG
#define TP_STATUS	(*(unsigned char*) 0177564)
#define TP_BYTE		(*(unsigned char*) 0177566)

void
debug_putchar (c)
	int c;
{
	for (;;) {
		/* Wait for console ready. */
		while (! (TP_STATUS & 0x80))
			continue;
		if (c == 0)
			return;

		/* Send byte. */
		TP_BYTE = c;

		if (c == '\n')
			c = '\r';
		else
			c = 0;
	}
}

/*
 * Printn prints a number n in base b.
 */
void debug_print_int(n)
	int n;
{
	register int i, d;

	i = 12;
	while (i >= 0) {
		d = n >> i;
		d &= 15;
		debug_putchar(d>9 ? d+'A'-10 : d+'0');
		i -= 4;
	}
}

/*
 * Scaled down version of C Library printf from 2.11BSD.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 */

/*VARARGS1*/
void
debug_printf(fmt, x1)
	char *fmt;
	unsigned int x1;
{
	register unsigned int *adx;
	register int c;
	int b;
	char *s;

	adx = &x1;
loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0')
			return;
		debug_putchar(c);
	}
	b = 8;
	c = *fmt++;
	switch (c) {
	case 'l':
		c = *fmt++;
		switch(c) {
		case 'x':
		case 'd':	/* only hex mode */
		case 'o':
			debug_print_int(*adx++);
			debug_print_int(*adx++);
			break;
		default:
			debug_putchar('%');
			debug_putchar('l');
			debug_putchar(c);
		}
		break;
	case 'x':
	case 'd':		/* only hex mode */
	case 'u':		/* what a joke */
	case 'o':
		debug_print_int(*adx++);
		break;
	case 'c':
		debug_putchar(*adx++);
		break;
	case 's':
		s = (char *)*adx++;
		while ((c = *s++))
			debug_putchar(c);
		break;
	case '%':
		debug_putchar(c);
		break;
	default:
		debug_putchar('%');
		debug_putchar(c);
		break;
	}
	goto loop;
}
#endif


#ifdef STANDALONE
/*
 * Compile standalone binary file.
 * Used for testing the compiler.
 */
void debug_printf();
void dbg_print_int();

void standalone ()
{
	debug_printf ("int=%x long=%lx\n", 0x1234, 0x5678abcdL);
	asm ("halt");
}
#endif

#ifdef DEBUG

/*
 * Printn prints a number n in base b.
 */
void dbg_print_int(n)
	int n;
{
	register int i, d;

	i = 12;
	while (i >= 0) {
		d = n >> i;
		d &= 15;
		dbg_putchar(d>9 ? d+'A'-10 : d+'0');
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
		dbg_putchar(c);
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
			dbg_print_int(*adx++);
			dbg_print_int(*adx++);
			break;
		default:
			dbg_putchar('%');
			dbg_putchar('l');
			dbg_putchar(c);
		}
		break;
	case 'x':
	case 'd':		/* only hex mode */
	case 'u':		/* what a joke */
	case 'o':
		dbg_print_int(*adx++);
		break;
	case 'c':
		dbg_putchar(*adx++);
		break;
	case 's':
		s = (char *)*adx++;
		while ((c = *s++))
			dbg_putchar(c);
		break;
	case '%':
		dbg_putchar(c);
		break;
	default:
		dbg_putchar('%');
		dbg_putchar(c);
		break;
	}
	goto loop;
}
#endif

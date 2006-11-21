char buf[6] = "  ";

int main ()
{
	register int i, j, c;

	for (i=0; i<16; ++i) {
		for (j=1; j<16; ++j) {
			if (j == 8 || j == 9)
				continue;
			c = j << 4 | i;

			switch (c) {
			case 021:
				buf [2] = buf [4] = 0202;
				break;
			case 020:
			case 022:
				c = ' ';
			default:
				/* Enable visible control characters. */
				buf [2] = buf [4] = 0204;
			}
			buf[3] = c;
			write (1, buf, 5);
		}
		write (1, "\r\n", 2);
	}
	return 0;
}

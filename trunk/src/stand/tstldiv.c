#ifndef __pdp11__
#define printhex(a)	printf ("%04x", (unsigned short) a)
#endif

void test (a, b, msg)
	long a, b;
	char *msg;
{
	long c;

	c = a / b;
	printhex ((int) (a >> 16)); printhex ((int) a);
	puts (" / ");
	printhex ((int) (b >> 16)); printhex ((int) b);
	puts (" = ");
	printhex ((int) (c >> 16)); printhex ((int) c);
	puts (" -- expected ");
	puts (msg);
	puts ("\n");
}

int main ()
{
	int a, b, c;

	puts ("Testing signed long division.\n");
	test (1234567L, 56789L, "00000015");
	test (-1234567L, 56789L, "ffffffeb");
	test (1234567L, -56789L, "ffffffeb");
	test (-1234567L, -56789L, "00000015");
	test (1234567L, 56L, "0000561d");
	test (1234L, 56L, "00000016");
	puts ("Done.\n");
	return 0;
}

#ifndef __pdp11__
#define printhex(a)	printf ("%04x", (unsigned short) a)
#endif

void test (a, b, msg)
	long a, b;
	char *msg;
{
	long c;

	c = a * b;
	printhex ((int) (a >> 16)); printhex ((int) a);
	puts (" * ");
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

	puts ("Testing signed long multiplication.\n");
	test (12345L, 6789L, "04fed79d");
	test (-12345L, 6789L, "fb012863");
	test (12345L, -6789L, "fb012863");
	test (-12345L, -6789L, "04fed79d");
	test (123L, 456789L, "035950d7");
	test (456789L, 123L, "035950d7");
	puts ("Done.\n");
	return 0;
}

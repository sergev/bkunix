#ifndef __pdp11__
#define printhex(a)	printf ("%04x", (unsigned short) a)
#endif

void test (a, b)
	long a, b;
{
	long c;

	c = a * b;
	printhex ((int) (a >> 16)); printhex ((int) a);
	printf (" * ");
	printhex ((int) (b >> 16)); printhex ((int) b);
	printf (" = ");
	printhex ((int) (c >> 16)); printhex ((int) c);
	printf ("\n");
}

int main ()
{
	int a, b, c;

	printf ("Testing integer multiplication.\n");
	test (12345L, 6789L);
	test (-12345L, 6789L);
	test (12345L, -6789L);
	test (-12345L, -6789L);
	test (123L, 456789L);
	test (456789L, 123L);
	printf ("Done.\n");
	return 0;
}

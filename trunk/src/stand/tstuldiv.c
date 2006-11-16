#ifndef __pdp11__
#define printhex(a)	printf ("%04x", a)
#endif

int main ()
{
	int a, b, c;

	printf ("Testing unsigned long division.\n");
	printf ("     ");
	for (b=2; b<10; ++b) {
		printf (" ");
		printhex (b);
	}
	printf ("\n");
	for (a=200; a<1000; a+=100) {
		printhex (a);
		printf (":");
		for (b=2; b<10; ++b) {
			c = (unsigned long) a / (unsigned long) b;
			printf (" ");
			printhex (c);
		}
		printf ("\n");
	}
	printf ("Done.\n");
	return 0;
}

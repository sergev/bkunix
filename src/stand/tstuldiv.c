#ifndef __pdp11__
#define printhex(a)	printf ("%04x", a)
#endif

int main ()
{
	int a, b, c;

	puts ("Testing unsigned long division.\n");
	puts ("     ");
	for (b=2; b<10; ++b) {
		puts (" ");
		printhex (b);
	}
	puts ("\n");
	for (a=200; a<1000; a+=100) {
		printhex (a);
		puts (":");
		for (b=2; b<10; ++b) {
			c = (unsigned long) a / (unsigned long) b;
			puts (" ");
			printhex (c);
		}
		puts ("\n");
	}
	puts ("Done.\n");
	return 0;
}

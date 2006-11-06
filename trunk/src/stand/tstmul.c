int main ()
{
	int a, b, c;

	printf ("Testing integer multiplication.\n");
	printf ("     ");
	for (b=2; b<10; ++b) {
		printf (" ");
		printhex (b);
	}
	printf ("\n");
	for (a=2; a<10; ++a) {
		printhex (a);
		printf (":");
		for (b=2; b<10; ++b) {
			c = a * b;
			printf (" ");
			printhex (c);
		}
		printf ("\n");
	}
	printf ("Done.\n");
	for (;;);
}

int main ()
{
	int i, val;

	printf ("Testing integer shift.\n");
	printhex (0x1234); putchar ('\n');
	printhex (0x4321); putchar ('\n');
	for (i=0; i<16; ++i) {
		printf ("1 << ");
		printhex (i);
		printf (" --> ");
		val = 1;
		val <<= i;
		printhex (val);

		printf (", 0x8000 >> ");
		printhex (i);
		printf (" --> ");
		val = 0x8000;
		val >>= i;
		printhex (val);
		printf ("\n");
	}
	printf ("Done.\n");
	for (;;);
}

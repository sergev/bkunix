int main ()
{
	int i, val;

	puts ("Testing integer shift.\n");
	printhex (0x1234); putchar ('\n');
	printhex (0x4321); putchar ('\n');
	for (i=0; i<16; ++i) {
		puts ("1 << ");
		printhex (i);
		puts (" --> ");
		val = 1;
		val <<= i;
		printhex (val);

		puts (", 0x8000 >> ");
		printhex (i);
		puts (" --> ");
		val = 0x8000;
		val >>= i;
		printhex (val);
		puts ("\n");
	}
	puts ("Done.\n");
	return 0;
}

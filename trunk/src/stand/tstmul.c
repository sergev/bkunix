int main ()
{
	int a, b, c;

	puts ("Testing integer multiplication.\n");
	puts ("     ");
	for (b=2; b<10; ++b) {
		puts (" ");
		printhex (b);
	}
	puts ("\n");
	for (a=2; a<10; ++a) {
		printhex (a);
		puts (":");
		for (b=2; b<10; ++b) {
			c = a * b;
			puts (" ");
			printhex (c);
		}
		puts ("\n");
	}
	puts ("Done.\n");
	return 0;
}

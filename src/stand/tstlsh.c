long lrol4();
asm (".text");
asm ("_lrol4: .globl _lrol4");
asm ("mov 4(sp),r1");
asm ("mov 2(sp),r0");
asm ("mov r2,-(sp)");
asm ("mov r0,r2");
asm ("rol r2");
asm ("rol r1");
asm ("rol r0");
asm ("rol r2");
asm ("rol r1");
asm ("rol r0");
asm ("rol r2");
asm ("rol r1");
asm ("rol r0");
asm ("rol r2");
asm ("rol r1");
asm ("rol r0");
asm ("mov (sp)+,r2");
asm ("rts pc");

void printlhex (val)
	long val;
{
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
	val = lrol4 (val);
	phexdigit ((int) val);
}

int main ()
{
	int i;
	long lval;

	puts ("Testing long shift.\n");
	printlhex (0x12345678); putchar ('\n');
	printlhex (0x87654321); putchar ('\n');
	for (i=0; i<16; ++i) {
		puts ("0x100 << ");
		printhex (i);
		puts (" --> ");
		lval = 0x100;
		lval <<= i;
		printlhex (lval);

		puts (", 0x800000 >> ");
		printhex (i);
		puts (" --> ");
		lval = 0x800000;
		lval >>= i;
		printlhex (lval);
		puts ("\n");
	}
	puts ("Done.\n");
	return 0;
}

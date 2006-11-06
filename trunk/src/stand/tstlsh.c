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
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
	val = lrol4 (val);
	phexgidit ((int) val);
}

int main ()
{
	int i;
	long lval;

	printf ("Testing long shift.\n");
	printlhex (0x12345678); putchar ('\n');
	printlhex (0x87654321); putchar ('\n');
	for (i=0; i<16; ++i) {
		printf ("0x100 << ");
		printhex (i);
		printf (" --> ");
		lval = 0x100;
		lval <<= i;
		printlhex (lval);

		printf (", 0x800000 >> ");
		printhex (i);
		printf (" --> ");
		lval = 0x800000;
		lval >>= i;
		printlhex (lval);
		printf ("\n");
	}
	printf ("Done.\n");
	for (;;);
}

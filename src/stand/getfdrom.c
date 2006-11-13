/*
 * Write floppy controller BIOS to floppy swap area.
 */
int bootdev;
char ioarea [64];

void fdinit ()
{
	/* When calling floppy BIOS, we must pass an address of
	 * i/o area in R3. Declare two register variables:
	 * the first is always placed in R4 by compiler,
	 * the second - in R3. */
	register char *r4, *r3;

	r3 = ioarea;
	((void(*)()) 0160010) ();
}

void fdstop ()
{
	/* Stop floppy motor. */
	*(int*) 0177130 = 0;
}

void fdwrite (mem, blk)
	char *mem;
	int blk;
{
	register int r4;
	register char *r3, *r2;

	printf ("Write buffer ");
	printhex (mem);
	printf (" to device ");
	printhex (bootdev);
	printf (" block ");
	printhex (blk);
	printf ("\n");

	r3 = ioarea;
	r3[034] = bootdev;
	r2 = mem;
	r4 = blk;

	asm("mov $-256, r1");	/* word cnt, negative for write */
	asm("mov r4, r0");	/* blk num */
	asm("mov r5,-(sp)");	/* r5 will be corrupted */
	asm("jsr pc, *$0160004");
	asm("bcs 1f");
	asm("mov (sp)+,r5");
	asm("jmp cret");
	asm("1: mov (sp)+,r5");

	printf (" -- Error.\n");
}

int main()
{
	register char *mem;
	register int blk;

	printf ("Get floppy controller ROM.\n");
	fdinit ();
	mem = (char*) 0160000;
	for (blk=0; blk<8; blk++) {
		fdwrite (mem, blk + 1600 - 128);
		mem += 512;
	}
	printf ("Done.\n");
	fdstop ();
	return 0;
}

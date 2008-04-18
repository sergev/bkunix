/*
 * Write floppy controller BIOS to floppy.
 */
int bootdev;

int main ()
{
	register int blk, ioarea, addr;

	/* Using BIOS i/o area at address 02000. */
	ioarea = 02000;		/* r3 */
	addr = 0160000;		/* r2 */
	blk = 80;		/* r4 */
	puts ("Get floppy controller ROM.\n");

	puts ("Write buffer ");
	printhex (addr);
	puts (" to device ");
	printhex (bootdev);
	puts (" block ");
	printhex (blk);
	puts ("\n");

	asm("mov $-2048, r1");	/* word cnt, negative for write */
	asm("mov r4, r0");	/* blk num */
	asm("mov r5,-(sp)");	/* r5 will be corrupted */
	asm("jsr pc, *$0160004");
	asm("bcs 1f");
	asm("mov (sp)+,r5");
	asm("jmp 2f");
	asm("1: mov (sp)+,r5");

	puts (" -- Error ");
	printhex (*(unsigned char*) 052);
	puts ("\n");

	asm("2:");
	puts ("Done.\n");

	/* Stop floppy motor. */
	*(int*) 0177130 = 0;
	return 0;
}

/*
 * Halt the Unix and go to BIOS monitor.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>

int
main()
{
	write(2, "System halted\n", 14);
	sync();
	sleep(2);

	/* Store "rts pc" instruction to prevent kernel restart. */
	*(int*) 0120000 = 0207;

	/* Stop floppy motor. */
	*(int*) 0177130 = 0;

	/* Jump to BIOS. */
	asm("jmp 0100000");
	return 0;
}

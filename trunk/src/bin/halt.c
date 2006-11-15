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
	asm("jbr 0100000");
	return 0;
}

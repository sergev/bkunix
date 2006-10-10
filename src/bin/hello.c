/*
 * Trivial user program.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>

char message[] = "Hello, World!\n";

int main ()
{
	write (0, message, sizeof(message) - 1);
	return 0;
}

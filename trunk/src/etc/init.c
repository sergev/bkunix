/*
 * Process control initialization.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>

char shell[] = "/bin/sh";
char minus[] = "-";
char ctty[] = "/dev/tty0";
char hello[] = "\n** bkunix **\n";
char failed[] = "\ninit: cannot exec /bin/sh\n";

int
main()
{
	open(ctty, 2);
	dup(0);
	dup(0);
	write(0, hello, sizeof(hello) - 1);
	chdir("/usr");
	open("/bin", 4);
	execl(shell, minus, 0);
	write(0, failed, sizeof(failed) - 1);
	return 0;
}

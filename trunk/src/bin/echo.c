/*
 * Write arguments to the standard output.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	int i;

	argc--;
	for (i=1; i<=argc; i++)
		printf("%s%c", argv[i], (i == argc) ? '\n': ' ');
	return 0;
}

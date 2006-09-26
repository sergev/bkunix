#include <stdlib.h>

extern int edata, end;

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

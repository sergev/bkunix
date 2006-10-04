#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	int m, a, b;

	if (argc != 5) {
usage:		write(1, "Usage: mknod name b/c major minor\n", 34);
		return 1;
	}
	if (*argv[2] == 'b')
		m = S_IFBLK;
	else if (*argv[2] == 'c')
		m = S_IFCHR;
	else
		goto usage;

	a = atoi(argv[3]);
	if (a < 0)
		goto usage;

	b = atoi(argv[4]);
	if (b < 0)
		goto usage;

	if (mknod(argv[1], m | 0666, (a << 8) | b) < 0) {
		write(2, "mknod failed\n", 13);
		return 1;
	}
	return 0;
}

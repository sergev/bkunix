/*
 * ln target [ new name ]
 */
#include <unistd.h>
#include <sys/stat.h>

int
main(argc, argv)
	char **argv;
{
	struct stat statb;
	register char *np;

	if (argc < 2) {
		write(1, "Usage: ln target [ newname ]\n", 29);
		return 1;
	}
	if (argc == 2) {
		np = argv[1];
		while (*np++);
		while (*--np != '/' && np > argv[1]);
		np++;
		argv[2] = np;
	}
	stat(argv[1], &statb);
	if (S_ISDIR(statb.st_mode)) {
		write(1, "No directory link\n", 18);
		return 1;
	}
	if (link(argv[1], argv[2]) < 0) {
		write(1, "Can't link\n", 11);
		return 1;
	}
	return 0;
}

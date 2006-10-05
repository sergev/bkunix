#include <string.h>
#include <unistd.h>

int
main(argc, argv)
	char **argv;
{
	sync();
	if (argc != 2) {
		write(2, "Usage: umount special\n", 22);
		return 1;
	}
	if (umount(argv[1]) < 0) {
		write(2, argv[1], strlen(argv[1]));
		write(2, ": umount failed\n", 16);
		return 1;
	}
	return 0;
}

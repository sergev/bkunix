#include <dirent.h>

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register int fd;

	fd = open(name, 0);
	if (fd < 0)
		return 0;
	return (DIR*) fd;
}

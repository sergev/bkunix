#include <dirent.h>

/*
 * close a directory.
 */
int
closedir(dirp)
	register DIR *dirp;
{
	return close((int) dirp);
}

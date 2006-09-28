#include <dirent.h>

/*
 * get next entry in a directory.
 */
struct dirent *
readdir(dirp)
	register DIR *dirp;
{
	static struct dirent d;

	while (read((int) dirp, &d, sizeof(d)) == sizeof(d)) {
		if (d.d_ino != 0)
			return &d;
	}
	return 0;
}

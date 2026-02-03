/*
 * Get next entry in a directory.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <dirent.h>

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

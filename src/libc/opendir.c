/*
 * Open a directory.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <dirent.h>

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

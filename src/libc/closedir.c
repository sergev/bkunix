/*
 * Close a directory.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <dirent.h>

int
closedir(dirp)
	register DIR *dirp;
{
	return close((int) dirp);
}

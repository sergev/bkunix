/*
 * Close a directory.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <dirent.h>

int
closedir(dirp)
	register DIR *dirp;
{
	return close((int) dirp);
}

/*
 * Open a filesystem, read superblock.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>
#include <string.h>
#include <sys/fs.h>

int
fs_open (fs, filename, writable)
	struct filesys *fs;
	char *filename;
	int writable;
{
	memset ((char*) fs, 0, sizeof (*fs));
	fs->filename = filename;
	fs->seek = 0;

	fs->fd = open (fs->filename, writable ? 2 : 0);
	if (fs->fd < 0)
		return 0;
	fs->writable = writable;

	if (! fs_seek (fs, 512L))
		return 0;

/*printf ("**superblock read\n");*/
	if (read (fs->fd, (char*) fs, FS_SUPERB_SIZE) != FS_SUPERB_SIZE)
		return 0;

	return 1;
}

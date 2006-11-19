/*
 * Seek disk for given offset.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>
#include <sys/fs.h>

int
fs_seek (fs, offset)
	struct filesys *fs;
	unsigned long offset;
{
	int r;

/*printf ("seek %ld, block %ld\n", offset, offset >> 9);*/
	r = lseek (fs->fd, offset, 0);
	if (r < 0) {
error:		printf ("error seeking %ld, block %ld\n",
			offset, offset >> 9);
		return 0;
	}
	fs->seek = offset;
	return 1;
}

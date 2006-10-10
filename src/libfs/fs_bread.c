/*
 * Read a block.
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
fs_bread (fs, bnum, data)
	struct filesys *fs;
	unsigned short bnum;
	unsigned char *data;
{
/*	printf ("read block %d\n", bnum);*/
	if (bnum <= fs->isize + 1)
		return 0;
	if (! fs_seek (fs, bnum * 512L))
		return 0;
	if (! fs_read (fs, data, 512))
		return 0;
	return 1;
}

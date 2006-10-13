/*
 * Read inode from disk.
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
fs_iget (fs, inode, inum)
	register struct filesys *fs;
	register struct inode *inode;
	unsigned short inum;
{
	unsigned long offset;

	memset (inode, 0, sizeof (*inode));
	inode->fs = fs;
	inode->number = inum;

	/* Inodes are numbered starting from 1.
	 * 32 bytes per inode, 16 inodes per block.
	 * Skip first and second blocks. */
	if (inum == 0 || inum > fs->isize*16)
		return 0;
	offset = (inode->number + 31) * 32;

	if (! fs_seek (fs, offset))
		return 0;

/*printf ("**inode %d read\n", inode->number);*/
	if (read (fs->fd, (char*) inode, FS_INODE_SIZE) != FS_INODE_SIZE)
		return 0;

	return 1;
}

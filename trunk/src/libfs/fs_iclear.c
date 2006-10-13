/*
 * Clear inode.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <string.h>
#include <sys/fs.h>

void
fs_iclear (inode)
	struct inode *inode;
{
	inode->dirty = 1;
	inode->mode = 0;
	inode->nlink = 0;
	inode->uid = 0;
	inode->size = 0;
	memset (inode->addr, 0, sizeof(inode->addr));
	inode->atime = 0;
	inode->mtime = 0;
}

/*
 * Add a block to free list.
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
fs_bfree (fs, bno)
	struct filesys *fs;
	unsigned int bno;
{
	int i;
	unsigned short buf [256];

/*	printf ("free block %d, total %d\n", bno, fs->nfree);*/
	if (fs->nfree >= 100) {
		buf[0] = fs->nfree;
		for (i=0; i<100; i++)
			buf[i+1] = fs->free[i];
		if (! fs_bwrite (fs, bno, (unsigned char*) buf)) {
			printf ("block_free: write error at block %d\n", bno);
			return 0;
		}
		fs->nfree = 0;
	}
	fs->free [fs->nfree] = bno;
	fs->nfree++;
	fs->dirty = 1;
	return 1;
}

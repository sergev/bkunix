/*
 * Print info about superblock.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>
#include <sys/fs.h>

void
fs_print (fs, verbose)
	struct filesys *fs;
{
	int i;

	printf ("                File: %s\n", fs->filename);
	printf ("         Volume size: %u blocks\n", fs->fsize);
	printf ("     Inode list size: %u blocks\n", fs->isize);
	printf ("   In-core free list: %u blocks", fs->nfree);
	if (verbose)
	    for (i=0; i < 100 && i < fs->nfree; ++i) {
		if (i % 10 == 0)
			printf ("\n                     ");
		printf (" %u", fs->free[i]);
	    }
	printf ("\n");

	printf (" In-core free inodes: %u inodes", fs->ninode);
	if (verbose)
	    for (i=0; i < 100 && i < fs->nfree; ++i) {
		if (i % 10 == 0)
			printf ("\n                     ");
		printf (" %u", fs->inode[i]);
	    }
	printf ("\n");
	if (verbose) {
		printf ("      Free list lock: %u\n", fs->flock);
		printf ("     Inode list lock: %u\n", fs->ilock);
		printf ("Super block modified: %u\n", fs->fmod);
		printf ("   Mounted read-only: %u\n", fs->ronly);
	}
	printf ("    Last update time: %s", ctime (&fs->time));
}

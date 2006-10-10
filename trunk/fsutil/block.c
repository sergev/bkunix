/*
 * Block handling for unix v6 filesystem.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "u6fs.h"

extern int verbose;

int u6fs_read_block (u6fs_t *fs, unsigned short bnum, unsigned char *data)
{
/*	printf ("read block %d\n", bnum);*/
	if (bnum <= fs->isize + 1)
		return 0;
	if (! u6fs_seek (fs, bnum * 512L))
		return 0;
	if (! u6fs_read (fs, data, 512))
		return 0;
	return 1;
}

int u6fs_write_block (u6fs_t *fs, unsigned short bnum, unsigned char *data)
{
/*	printf ("write block %d\n", bnum);*/
	if (! fs->writable || bnum <= fs->isize + 1)
		return 0;
	if (! u6fs_seek (fs, bnum * 512L))
		return 0;
	if (! u6fs_write (fs, data, 512))
		return 0;
	fs->modified = 1;
	return 1;
}

/*
 * Add a block to free list.
 */
int u6fs_block_free (u6fs_t *fs, unsigned int bno)
{
	int i;
	unsigned short buf [256];

/*	printf ("free block %d, total %d\n", bno, fs->nfree);*/
	if (fs->nfree >= 100) {
		buf[0] = lsb_short (fs->nfree);
		for (i=0; i<100; i++)
			buf[i+1] = lsb_short (fs->free[i]);
		if (! u6fs_write_block (fs, bno, (char*) buf)) {
			fprintf (stderr, "block_free: write error at block %d\n", bno);
			return 0;
		}
		fs->nfree = 0;
	}
	fs->free [fs->nfree] = bno;
	fs->nfree++;
	fs->dirty = 1;
	return 1;
}

/*
 * Free an indirect block.
 */
int u6fs_indirect_block_free (u6fs_t *fs, unsigned int bno)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	if (! u6fs_read_block (fs, bno, data)) {
		fprintf (stderr, "inode_clear: read error at block %d\n", bno);
		return 0;
	}
	for (i=LSXFS_BSIZE-2; i>=0; i-=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			u6fs_block_free (fs, nb);
	}
	u6fs_block_free (fs, bno);
	return 1;
}

/*
 * Free a double indirect block.
 */
int u6fs_double_indirect_block_free (u6fs_t *fs, unsigned int bno)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	if (! u6fs_read_block (fs, bno, data)) {
		fprintf (stderr, "inode_clear: read error at block %d\n", bno);
		return 0;
	}
	for (i=LSXFS_BSIZE-2; i>=0; i-=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			u6fs_indirect_block_free (fs, nb);
	}
	u6fs_block_free (fs, bno);
	return 1;
}

/*
 * Get a block from free list.
 */
int u6fs_block_alloc (u6fs_t *fs, unsigned int *bno)
{
	int i;
	unsigned short buf [256];
again:
	if (fs->nfree == 0)
		return 0;
	fs->nfree--;
	*bno = fs->free [fs->nfree];
/*	printf ("allocate new block %d from slot %d\n", *bno, fs->nfree);*/
	fs->free [fs->nfree] = 0;
	fs->dirty = 1;
	if (fs->nfree <= 0) {
		if (! u6fs_read_block (fs, *bno, (char*) buf))
			return 0;
		fs->nfree = lsb_short (buf[0]);
		for (i=0; i<100; i++)
			fs->free[i] = lsb_short (buf[i+1]);
	}
	if (*bno == 0)
		goto again;
	return 1;
}

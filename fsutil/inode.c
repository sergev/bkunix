/*
 * Inode routines for unix v6 filesystem.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "u6fs.h"

extern int verbose;

int u6fs_inode_get (u6fs_t *fs, u6fs_inode_t *inode, unsigned short inum)
{
	unsigned long offset;
	int i;

	memset (inode, 0, sizeof (*inode));
	inode->fs = fs;
	inode->number = inum;

	/* Inodes are numbered starting from 1.
	 * 32 bytes per inode, 16 inodes per block.
	 * Skip first and second blocks. */
	if (inum == 0 || inum > fs->isize*16)
		return 0;
	offset = (inode->number + 31) * 32;

	if (! u6fs_seek (fs, offset))
		return 0;

	if (! u6fs_read16 (fs, &inode->mode))	/* file type and access mode */
		return 0;
	if (! u6fs_read8 (fs, &inode->nlink))	/* directory entries */
		return 0;
	if (! u6fs_read8 (fs, &inode->uid))	/* owner */
		return 0;
	if (! u6fs_read32 (fs, &inode->size))	/* size */
		return 0;

	for (i=0; i<8; ++i) {		/* device addresses constituting file */
		if (! u6fs_read16 (fs, &inode->addr[i]))
			return 0;
	}
	if (! u6fs_read32 (fs, &inode->atime))	/* last access time */
		return 0;
	if (! u6fs_read32 (fs, &inode->mtime))	/* last modification time */
		return 0;
	return 1;
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
void u6fs_inode_truncate (u6fs_inode_t *inode)
{
	unsigned short *blk;

	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FCHR ||
	    (inode->mode & INODE_MODE_FMT) == INODE_MODE_FBLK)
		return;

	for (blk = &inode->addr[7]; blk >= &inode->addr[0]; --blk) {
		if (*blk == 0)
			continue;

		if (! (inode->mode & INODE_MODE_LARG))
			u6fs_block_free (inode->fs, *blk);
		else if (blk == &inode->addr[7])
			u6fs_double_indirect_block_free (inode->fs, *blk);
		else
			u6fs_indirect_block_free (inode->fs, *blk);

		*blk = 0;
	}
	inode->mode &= ~INODE_MODE_LARG;
	inode->size = 0;
	inode->dirty = 1;
}

void u6fs_inode_clear (u6fs_inode_t *inode)
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

int u6fs_inode_save (u6fs_inode_t *inode, int force)
{
	unsigned long offset;
	int i;

	if (! inode->fs->writable)
		return 0;
	if (! force && ! inode->dirty)
		return 1;
	if (inode->number == 0 || inode->number > inode->fs->isize*16)
		return 0;
	offset = (inode->number + 31) * 32;

	time (&inode->atime);
	time (&inode->mtime);

	if (! u6fs_seek (inode->fs, offset))
		return 0;

	if (! u6fs_write16 (inode->fs, inode->mode))	/* file type and access mode */
		return 0;
	if (! u6fs_write8 (inode->fs, inode->nlink))	/* directory entries */
		return 0;
	if (! u6fs_write8 (inode->fs, inode->uid))	/* owner */
		return 0;
	if (! u6fs_write32 (inode->fs, inode->size))	/* size */
		return 0;

	for (i=0; i<8; ++i) {		/* device addresses constituting file */
		if (! u6fs_write16 (inode->fs, inode->addr[i]))
			return 0;
	}
	if (! u6fs_write32 (inode->fs, inode->atime))	/* last access time */
		return 0;
	if (! u6fs_write32 (inode->fs, inode->mtime))	/* last modification time */
		return 0;

	inode->dirty = 0;
	return 1;
}

void u6fs_inode_print (u6fs_inode_t *inode, FILE *out)
{
	int i;

	fprintf (out, "     I-node: %u\n", inode->number);
	fprintf (out, "       Type: %s\n",
		(inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR ? "Directory" :
		(inode->mode & INODE_MODE_FMT) == INODE_MODE_FCHR ? "Character device" :
		(inode->mode & INODE_MODE_FMT) == INODE_MODE_FBLK ? "Block device" :
		"File");
	fprintf (out, "       Size: %lu bytes\n", inode->size);
	fprintf (out, "       Mode: %#o\n", inode->mode);

	fprintf (out, "            ");
	if (inode->mode & INODE_MODE_ALLOC) fprintf (out, " ALLOC");
        if (inode->mode & INODE_MODE_LARG)  fprintf (out, " LARG");
        if (inode->mode & INODE_MODE_SUID)  fprintf (out, " SUID");
        if (inode->mode & INODE_MODE_SGID)  fprintf (out, " SGID");
        if (inode->mode & INODE_MODE_SVTX)  fprintf (out, " SVTX");
        if (inode->mode & INODE_MODE_READ)  fprintf (out, " READ");
        if (inode->mode & INODE_MODE_WRITE) fprintf (out, " WRITE");
        if (inode->mode & INODE_MODE_EXEC)  fprintf (out, " EXEC");
	fprintf (out, "\n");

	fprintf (out, "      Links: %u\n", inode->nlink);
	fprintf (out, "   Owner id: %u\n", inode->uid);

	fprintf (out, "     Blocks:");
	for (i=0; i < 8; ++i) {
		fprintf (out, " %u", inode->addr[i]);
	}
	fprintf (out, "\n");

	fprintf (out, "Last access: %s", ctime (&inode->atime));
	fprintf (out, "   Modified: %s", ctime (&inode->mtime));
}

void u6fs_directory_scan (u6fs_inode_t *dir, char *dirname,
	u6fs_directory_scanner_t scanner, void *arg)
{
	u6fs_inode_t file;
	unsigned long offset;
	unsigned char data [17];
	unsigned int inum;

	/* 16 bytes per file */
	for (offset = 0; dir->size - offset >= 16; offset += 16) {
		if (! u6fs_inode_read (dir, offset, data, 16)) {
			fprintf (stderr, "%s: read error at offset %ld\n",
				dirname[0] ? dirname : "/", offset);
			return;
		}
		data [16] = 0;
		inum = data [1] << 8 | data [0];

		if (inum == 0 || (data[2]=='.' && data[3]==0) ||
		    (data[2]=='.' && data[3]=='.' && data[4]==0))
			continue;

		if (! u6fs_inode_get (dir->fs, &file, inum)) {
			fprintf (stderr, "cannot scan inode %d\n", inum);
			continue;
		}
		scanner (dir, &file, dirname, &data[2], arg);
	}
}

/*
 * Return the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned short map_block (u6fs_inode_t *inode, unsigned short lbn)
{
	unsigned char block [512];
	unsigned int nb, i;

	if (lbn > 0x7fff) {
		/* block number too large */
		return 0;
	}
	if (! (inode->mode & INODE_MODE_LARG)) {
		/* small file algorithm */
		if (lbn > 7) {
			/* block number too large for small file */
			return 0;
		}
		return inode->addr [lbn];
	}

	/* large file algorithm */
	i = lbn >> 8;
	if (i > 7)
		i = 7;
	nb = inode->addr [i];
	if (nb == 0)
		return 0;
	if (! u6fs_read_block (inode->fs, nb, block))
		return 0;

	/* "huge" fetch of double indirect block */
	if (i == 7) {
		i = ((lbn >> 8) - 7) * 2;
		nb = block [i+1] << 8 | block [i];
		if (nb == 0)
			return 0;
		if (! u6fs_read_block (inode->fs, nb, block))
			return 0;
	}

	/* normal indirect fetch */
	i = (lbn & 0xFF) * 2;
	nb = block [i+1] << 8 | block [i];
	return nb;
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned short map_block_write (u6fs_inode_t *inode, unsigned short lbn)
{
	unsigned char block [512];
	unsigned int nb, ib, i;

	if (lbn > 0x7fff) {
		/* block number too large */
		return 0;
	}
	if (! (inode->mode & INODE_MODE_LARG)) {
		/* small file algorithm */
		if (lbn > 7) {
			/* convert small to large */
			if (! u6fs_block_alloc (inode->fs, &nb))
				return 0;
			memset (block, 0, 512);
			for (i=0; i<8; i++) {
				block[i+i] = inode->addr[i];
				block[i+i+1] = inode->addr[i] >> 8;
				inode->addr[i] = 0;
			}
			inode->addr[0] = nb;
			if (! u6fs_write_block (inode->fs, nb, block))
				return 0;
			inode->mode |= INODE_MODE_LARG;
			inode->dirty = 1;
			goto large;
		}
		nb = inode->addr[lbn];
		if (nb != 0) {
/*			printf ("map logical block %d to physical %d\n", lbn, nb);*/
			return nb;
		}

		/* allocate new block */
		if (! u6fs_block_alloc (inode->fs, &nb))
			return 0;
		inode->addr[lbn] = nb;
		inode->dirty = 1;
		return nb;
	}
large:
	/* large file algorithm */
	i = lbn >> 8;
	if (i > 7)
		i = 7;
	ib = inode->addr[i];
	if (ib != 0) {
		if (! u6fs_read_block (inode->fs, ib, block))
			return 0;
	} else {
		if (! u6fs_block_alloc (inode->fs, &ib))
			return 0;
		memset (block, 0, 512);
		inode->addr[i] = ib;
		inode->dirty = 1;
	}

	/* "huge" fetch of double indirect block */
	if (i == 7) {
		i = ((lbn >> 8) - 7) * 2;
		nb = block [i+1] << 8 | block [i];
		if (nb != 0) {
			if (! u6fs_read_block (inode->fs, nb, block))
				return 0;
		} else {
			/* allocate new block */
			if (! u6fs_block_alloc (inode->fs, &nb))
				return 0;
			memset (block, 0, 512);
			block[i+i] = nb;
			block[i+i+1] = nb >> 8;
			if (! u6fs_write_block (inode->fs, ib, block))
				return 0;
		}
		ib = nb;
	}

	/* normal indirect fetch */
	i = lbn & 0377;
	nb = block [i+i+1] << 8 | block [i+i];
	if (nb != 0)
		return nb;

	/* allocate new block */
	if (! u6fs_block_alloc (inode->fs, &nb))
		return 0;
/*	printf ("inode %d: allocate new block %d\n", inode->number, nb);*/
	block[i+i] = nb;
	block[i+i+1] = nb >> 8;
	if (! u6fs_write_block (inode->fs, ib, block))
		return 0;
	return nb;
}

int u6fs_inode_read (u6fs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes)
{
	unsigned char block [512];
	unsigned long n;
	unsigned int bn, inblock_offset;

	if (bytes + offset > inode->size)
		return 0;
	while (bytes != 0) {
		inblock_offset = offset % 512;
		n = 512 - inblock_offset;
		if (n > bytes)
			n = bytes;

		bn = map_block (inode, offset / 512);
		if (bn == 0)
			return 0;

		if (! u6fs_read_block (inode->fs, bn, block))
			return 0;
		memcpy (data, block + inblock_offset, n);
		offset += n;
		bytes -= n;
	}
	return 1;
}

int u6fs_inode_write (u6fs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes)
{
	unsigned char block [512];
	unsigned long n;
	unsigned int bn, inblock_offset;

	while (bytes != 0) {
		inblock_offset = offset % 512;
		n = 512 - inblock_offset;
		if (n > bytes)
			n = bytes;

		bn = map_block_write (inode, offset / 512);
		if (bn == 0)
			return 0;
		if (inode->size < offset + n) {
			/* Increase file size. */
			inode->size = offset + n;
			inode->dirty = 1;
		}
		if (verbose)
			printf ("inode %d offset %ld: write %ld bytes to block %d\n",
				inode->number, offset, n, bn);

		if (n == 512) {
			if (! u6fs_write_block (inode->fs, bn, data))
				return 0;
		} else {
			if (! u6fs_read_block (inode->fs, bn, block))
				return 0;
			memcpy (block + inblock_offset, data, n);
			if (! u6fs_write_block (inode->fs, bn, block))
				return 0;
		}
		offset += n;
		bytes -= n;
	}
	return 1;
}

/*
 * Convert from dirent to raw data.
 */
void u6fs_dirent_pack (unsigned char *data, u6fs_dirent_t *dirent)
{
	int i;

	*data++ = dirent->ino;
	*data++ = dirent->ino >> 16;
	for (i=0; i<14 && dirent->name[i]; ++i)
		*data++ = dirent->name[i];
	for (; i<14; ++i)
		*data++ = 0;
}

/*
 * Read dirent from raw data.
 */
void u6fs_dirent_unpack (u6fs_dirent_t *dirent, unsigned char *data)
{
	int i;

	dirent->ino = *data++;
	dirent->ino |= *data++ << 8;
	for (i=0; i<14; ++i)
		dirent->name[i] = *data++;
	dirent->name[14] = 0;
}

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * flag = 0 if name is saught
 *	1 if name is to be created, mode is given
 *	2 if name is to be deleted
 *	3 if name is to be linked, mode contains inode number
 */
int u6fs_inode_by_name (u6fs_t *fs, u6fs_inode_t *inode, char *name,
	int op, int mode)
{
	int c;
	char *cp;
	char dbuf [14];
	unsigned long offset;
	unsigned char data [16];
	unsigned int inum;
	u6fs_inode_t dir;

	/* Start from root. */
	if (! u6fs_inode_get (fs, &dir, LSXFS_ROOT_INODE)) {
		fprintf (stderr, "inode_open(): cannot get root\n");
		return 0;
	}
	c = *name++;
	while (c == '/')
		c = *name++;
	if (! c && op != 0) {
		/* Cannot write or delete root directory. */
		return 0;
	}
cloop:
	/* Here inode contains pointer
	 * to last component matched. */
	if (! c) {
		*inode = dir;
		return 1;
	}

	/* If there is another component,
	 * inode must be a directory. */
	if ((dir.mode & INODE_MODE_FMT) != INODE_MODE_FDIR) {
		return 0;
	}

	/* Gather up dir name into buffer. */
	cp = &dbuf[0];
	while (c && c != '/') {
		if (cp < dbuf + sizeof(dbuf))
			*cp++ = c;
		c = *name++;
	}
	while (cp < dbuf + sizeof(dbuf))
		*cp++ = 0;
	while (c == '/')
		c = *name++;

	/* Search a directory, 16 bytes per file */
	for (offset = 0; dir.size - offset >= 16; offset += 16) {
		if (! u6fs_inode_read (&dir, offset, data, 16)) {
			fprintf (stderr, "inode %d: read error at offset %ld\n",
				dir.number, offset);
			return 0;
		}
		inum = data [1] << 8 | data [0];
		if (inum == 0)
			continue;
		if (strncmp (dbuf, data+2, 14) == 0) {
			/* Here a component matched in a directory.
			 * If there is more pathname, go back to
			 * cloop, otherwise return. */
			if (op == 2 && ! c) {
				goto delete_file;
			}
			if (! u6fs_inode_get (fs, &dir, inum)) {
				fprintf (stderr, "inode_open(): cannot get inode %d\n", inum);
				return 0;
			}
			goto cloop;
		}
	}
	/* If at the end of the directory,
	 * the search failed. Report what
	 * is appropriate as per flag. */
	if (op == 1 && ! c)
		goto create_file;
	if (op == 3 && ! c)
		goto create_link;
	return 0;

	/*
	 * Make a new file, and return it's inode.
	 */
create_file:
	if (! u6fs_inode_alloc (fs, inode)) {
		fprintf (stderr, "%s: cannot allocate inode\n", name);
		return 0;
	}
	inode->dirty = 1;
	inode->mode = mode & (07777 | INODE_MODE_FMT);
	inode->mode |= INODE_MODE_ALLOC;
	inode->nlink = 1;
	inode->uid = 0;
	if (! u6fs_inode_save (inode, 0)) {
		fprintf (stderr, "%s: cannot save file inode\n", name);
		return 0;
	}

	/* Write a directory entry. */
	data[0] = inode->number;
	data[1] = inode->number >> 8;
	memcpy (data+2, dbuf, 14);
write_back:
	if (! u6fs_inode_write (&dir, offset, data, 16)) {
		fprintf (stderr, "inode %d: write error at offset %ld\n",
			inode->number, offset);
		return 0;
	}
	if (! u6fs_inode_save (&dir, 0)) {
		fprintf (stderr, "%s: cannot save directory inode\n", name);
		return 0;
	}
	return 1;

	/*
	 * Delete file. Return inode of deleted file.
	 */
delete_file:
	if (! u6fs_inode_get (fs, inode, inum)) {
		fprintf (stderr, "%s: cannot get inode %d\n", name, inum);
		return 0;
	}
	inode->dirty = 1;
	inode->nlink--;
	if (inode->nlink <= 0) {
		u6fs_inode_truncate (inode);
		u6fs_inode_clear (inode);
		if (inode->fs->ninode < 100) {
			inode->fs->inode [inode->fs->ninode++] = inum;
			inode->fs->dirty = 1;
		}
	}
	memset (data, 0, 16);
	goto write_back;

	/*
	 * Make a link. Return a directory inode.
	 */
create_link:
	data[0] = mode;
	data[1] = mode >> 8;
/*printf ("*** link inode %d to %s\n", mode, dbuf);*/
	memcpy (data+2, dbuf, 14);
/*printf ("*** add entry '%.14s' to inode %d\n", dbuf, dir.number);*/
	if (! u6fs_inode_write (&dir, offset, data, 16)) {
		fprintf (stderr, "inode %d: write error at offset %ld\n",
			dir.number, offset);
		return 0;
	}
	if (! u6fs_inode_save (&dir, 0)) {
		fprintf (stderr, "%s: cannot save directory inode\n", name);
		return 0;
	}
	*inode = dir;
	return 1;
}

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * 100 spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up 100 more.
 */
int u6fs_inode_alloc (u6fs_t *fs, u6fs_inode_t *inode)
{
	int ino;

	for (;;) {
		if (fs->ninode <= 0) {
			return 0;
		}
		ino = fs->inode[--fs->ninode];
		fs->dirty = 1;
		if (! u6fs_inode_get (fs, inode, ino)) {
			fprintf (stderr, "inode_alloc: cannot get inode %d\n", ino);
			return 0;
		}
		if (inode->mode == 0) {
			u6fs_inode_clear (inode);
			return 1;
		}
	}
}

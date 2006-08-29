#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "lsxfs.h"

extern int verbose;

int lsxfs_inode_get (lsxfs_t *fs, lsxfs_inode_t *inode, unsigned short inum)
{
	unsigned long offset;
	unsigned char size2;
	unsigned short size10;
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

	if (! lsxfs_seek (fs, offset))
		return 0;

	if (! lsxfs_read16 (fs, &inode->mode))	/* file type and access mode */
		return 0;
	if (! lsxfs_read8 (fs, &inode->nlink))	/* directory entries */
		return 0;
	if (! lsxfs_read8 (fs, &inode->uid))	/* owner */
		return 0;
	if (! lsxfs_read8 (fs, &inode->gid))	/* group of owner */
		return 0;

	/* size */
	if (! lsxfs_read8 (fs, &size2))
		return 0;
	if (! lsxfs_read16 (fs, &size10))
		return 0;
	inode->size = (unsigned long) size2 << 16 | size10;

	for (i=0; i<8; ++i) {		/* device addresses constituting file */
		if (! lsxfs_read16 (fs, &inode->addr[i]))
			return 0;
	}
	if (! lsxfs_read32 (fs, &inode->atime))	/* last access time */
		return 0;
	if (! lsxfs_read32 (fs, &inode->mtime))	/* last modification time */
		return 0;
	return 1;
}

void lsxfs_inode_clear (lsxfs_inode_t *inode)
{
	inode->dirty = 1;
	inode->mode = 0;
	inode->nlink = 0;
	inode->uid = 0;
	inode->gid = 0;
	inode->size = 0;
	memset (inode->addr, 0, sizeof(inode->addr));
	inode->atime = 0;
	inode->mtime = 0;
}

int lsxfs_inode_save (lsxfs_inode_t *inode)
{
	unsigned long offset;
	int i;

	if (inode->number == 0 || inode->number > inode->fs->isize*16)
		return 0;
	offset = (inode->number + 31) * 32;

	if (! lsxfs_seek (inode->fs, offset))
		return 0;

	if (! lsxfs_write16 (inode->fs, inode->mode))	/* file type and access mode */
		return 0;
	if (! lsxfs_write8 (inode->fs, inode->nlink))	/* directory entries */
		return 0;
	if (! lsxfs_write8 (inode->fs, inode->uid))	/* owner */
		return 0;
	if (! lsxfs_write8 (inode->fs, inode->gid))	/* group of owner */
		return 0;

	/* size */
	if (! lsxfs_write8 (inode->fs, inode->size >> 16))
		return 0;
	if (! lsxfs_write16 (inode->fs, inode->size))
		return 0;

	for (i=0; i<8; ++i) {		/* device addresses constituting file */
		if (! lsxfs_write16 (inode->fs, inode->addr[i]))
			return 0;
	}
	if (! lsxfs_write32 (inode->fs, inode->atime))	/* last access time */
		return 0;
	if (! lsxfs_write32 (inode->fs, inode->mtime))	/* last modification time */
		return 0;

	inode->dirty = 0;
	return 1;
}

void lsxfs_inode_print (lsxfs_inode_t *inode, FILE *out)
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
	fprintf (out, "   Group id: %u\n", inode->gid);

	fprintf (out, "     Blocks:");
	for (i=0; i < 8; ++i) {
		fprintf (out, " %u", inode->addr[i]);
	}
	fprintf (out, "\n");

	fprintf (out, "Last access: %s", ctime (&inode->atime));
	fprintf (out, "   Modified: %s", ctime (&inode->mtime));
}

void lsxfs_directory_scan (lsxfs_inode_t *dir, char *dirname,
	lsxfs_directory_scanner_t scanner, void *arg)
{
	lsxfs_inode_t file;
	unsigned long offset;
	unsigned char data [17];
	unsigned int inum;

	/* 16 bytes per file */
	for (offset = 0; dir->size - offset >= 16; offset += 16) {
		if (! lsxfs_file_read (dir, offset, data, 16)) {
			fprintf (stderr, "%s: read error at offset %ld\n",
				dirname[0] ? dirname : "/", offset);
			return;
		}
		data [16] = 0;
		inum = data [1] << 8 | data [0];

		if (inum == 0 || (data[2]=='.' && data[3]==0) ||
		    (data[2]=='.' && data[3]=='.' && data[4]==0))
			continue;

		if (! lsxfs_inode_get (dir->fs, &file, inum)) {
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
static unsigned short map_block (lsxfs_inode_t *inode, unsigned short lbn)
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
	if (! lsxfs_read_block (inode->fs, nb, block))
		return 0;

	/* "huge" fetch of double indirect block */
	if (i == 7) {
		i = ((lbn >> 8) - 7) * 2;
		nb = block [i+1] << 8 | block [i];
		if (nb == 0)
			return 0;
		if (! lsxfs_read_block (inode->fs, nb, block))
			return 0;
	}

	/* normal indirect fetch */
	i = (lbn & 0xFF) * 2;
	nb = block [i+1] << 8 | block [i];
	return nb;
}

int lsxfs_file_read (lsxfs_inode_t *inode, unsigned long offset,
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

		if (! lsxfs_read_block (inode->fs, bn, block))
			return 0;
		memcpy (data, block + inblock_offset, n);
		offset += n;
		bytes -= n;
	}
	return 1;
}

/*
 * Convert from dirent to raw data.
 */
void lsxfs_dirent_pack (unsigned char *data, lsxfs_dirent_t *dirent)
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
void lsxfs_dirent_unpack (lsxfs_dirent_t *dirent, unsigned char *data)
{
	int i;

	dirent->ino = *data++;
	dirent->ino |= *data++ << 8;
	for (i=0; i<14; ++i)
		dirent->name[i] = *data++;
	dirent->name[14] = 0;
}

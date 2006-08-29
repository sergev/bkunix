#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "lsxfs.h"

extern int verbose;

static unsigned long deskew (unsigned long address)
{
	unsigned long track, sector;
	unsigned int offset = address % 128;

	sector = (address / 128) * 3 % 26;
	track = (address / 128) / 26 + 1;
	if (track == 77)
		track = 0;
	return (track * 26 + sector) * 128 + offset;
}

int lsxfs_seek (lsxfs_t *fs, unsigned long offset)
{
	unsigned long hw_address;

	hw_address = deskew (offset);
/*	printf ("seek %ld, block %ld - hw %d\n", offset, offset / 512, hw_address);*/
	if (lseek (fs->fd, hw_address, 0) < 0) {
		if (verbose)
			printf ("error seeking %ld, block %ld - hw %ld\n",
				offset, offset / 512, hw_address);
		return 0;
	}
	fs->seek = offset;
	return 1;
}

static void update_seek (lsxfs_t *fs, unsigned int offset)
{
	/* Update current seek position.
	 * Called after read/write.
	 * When out of 128-byte sector - seek needed. */
	if (fs->seek % 128 + offset >= 128)
		lsxfs_seek (fs, fs->seek + offset);
	else
		fs->seek += offset;
}

int lsxfs_read8 (lsxfs_t *fs, unsigned char *val)
{
	if (read (fs->fd, val, 1) != 1) {
		if (verbose)
			printf ("error read8, seek %ld block %ld\n", fs->seek, fs->seek / 512);
		return 0;
	}
	update_seek (fs, 1);
	return 1;
}

int lsxfs_read16 (lsxfs_t *fs, unsigned short *val)
{
	unsigned char data [2];

	if (read (fs->fd, data, 2) != 2) {
		if (verbose)
			printf ("error read16, seek %ld block %ld\n", fs->seek, fs->seek / 512);
		return 0;
	}
	update_seek (fs, 2);
	*val = data[1] << 8 | data[0];
	return 1;
}

int lsxfs_read32 (lsxfs_t *fs, unsigned long *val)
{
	unsigned char data [4];

	if (read (fs->fd, data, 4) != 4) {
		if (verbose)
			printf ("error read32, seek %ld block %ld\n", fs->seek, fs->seek / 512);
		return 0;
	}
	update_seek (fs, 4);
	*val = (unsigned long) data[1] << 24 | (unsigned long) data[0] << 16 |
		data[3] << 8 | data[2];
	return 1;
}

int lsxfs_write8 (lsxfs_t *fs, unsigned char val)
{
	if (write (fs->fd, &val, 1) != 1)
		return 0;
	update_seek (fs, 1);
	return 1;
}

int lsxfs_write16 (lsxfs_t *fs, unsigned short val)
{
	unsigned char data [2];

	data[0] = val;
	data[1] = val >> 8;
	if (write (fs->fd, data, 2) != 2)
		return 0;
	update_seek (fs, 2);
	return 1;
}

int lsxfs_write32 (lsxfs_t *fs, unsigned long val)
{
	unsigned char data [4];

	data[0] = val >> 16;
	data[1] = val >> 24;
	data[2] = val;
	data[3] = val >> 8;
	if (write (fs->fd, data, 4) != 4)
		return 0;
	update_seek (fs, 4);
	return 1;
}

int lsxfs_read_block (lsxfs_t *fs, unsigned short bnum, unsigned char *data)
{
	int i;

/*	printf ("read block %d\n", bnum);*/
	if (bnum <= fs->isize + 1)
		return 0;
	if (! lsxfs_seek (fs, bnum * 512L))
		return 0;
	for (i=0; i<4; ++i) {
		if (read (fs->fd, data, 128) != 128)
			return 0;
		update_seek (fs, 128);
		data += 128;
	}
	return 1;
}

int lsxfs_write (lsxfs_t *fs, unsigned char *data, int bytes)
{
	int len;

	if (! fs->writable)
		return 0;
	while (bytes > 0) {
		len = bytes;
		if (len > 128)
			len = 128;
		if (write (fs->fd, data, len) != len)
			return 0;
		update_seek (fs, len);
		data += len;
		bytes -= len;
	}
	return 1;
}

int lsxfs_write_block (lsxfs_t *fs, unsigned short bnum, unsigned char *data)
{
/*	printf ("write block %d\n", bnum);*/
	if (! fs->writable || bnum <= fs->isize + 1)
		return 0;
	if (! lsxfs_seek (fs, bnum * 512L))
		return 0;
	if (! lsxfs_write (fs, data, 512))
		return 0;
	fs->modified = 1;
	return 1;
}

int lsxfs_open (lsxfs_t *fs, const char *filename, int writable)
{
	int i;

	memset (fs, 0, sizeof (*fs));
	fs->filename = filename;
	fs->seek = 0;

	fs->fd = open (fs->filename, writable ? O_RDWR : O_RDONLY);
	if (fs->fd < 0)
		return 0;
	fs->writable = writable;

	if (! lsxfs_seek (fs, 512))
		return 0;

	if (! lsxfs_read16 (fs, &fs->isize))	/* size in blocks of I list */
		return 0;
	if (! lsxfs_read16 (fs, &fs->fsize))	/* size in blocks of entire volume */
		return 0;
	if (! lsxfs_read16 (fs, &fs->nfree))	/* number of in core free blocks (0-100) */
		return 0;
	for (i=0; i<100; ++i) {			/* in core free blocks */
		if (! lsxfs_read16 (fs, &fs->free[i]))
			return 0;
	}
	if (! lsxfs_read16 (fs, &fs->ninode))	/* number of in core I nodes (0-100) */
		return 0;
	for (i=0; i<100; ++i) {			/* in core free I nodes */
		if (! lsxfs_read16 (fs, &fs->inode[i]))
			return 0;
	}
	if (! lsxfs_read8 (fs, &fs->flock))	/* lock during free list manipulation */
		return 0;
	if (! lsxfs_read8 (fs, &fs->ilock))	/* lock during I list manipulation */
		return 0;
	if (! lsxfs_read8 (fs, &fs->fmod))	/* super block modified flag */
		return 0;
	if (! lsxfs_read8 (fs, &fs->ronly))	/* mounted read-only flag */
		return 0;
	if (! lsxfs_read32 (fs, &fs->time))	/* current date of last update */
		return 0;
	return 1;
}

int lsxfs_sync (lsxfs_t *fs, int force)
{
	int i;

	if (! fs->writable)
		return 0;
	if (! force && ! fs->dirty)
		return 1;
	if (! lsxfs_seek (fs, 512))
		return 0;

	if (! lsxfs_write16 (fs, fs->isize))	/* size in blocks of I list */
		return 0;
	if (! lsxfs_write16 (fs, fs->fsize))	/* size in blocks of entire volume */
		return 0;
	if (! lsxfs_write16 (fs, fs->nfree))	/* number of in core free blocks (0-100) */
		return 0;
	for (i=0; i<100; ++i) {			/* in core free blocks */
		if (! lsxfs_write16 (fs, fs->free[i]))
			return 0;
	}
	if (! lsxfs_write16 (fs, fs->ninode))	/* number of in core I nodes (0-100) */
		return 0;
	for (i=0; i<100; ++i) {			/* in core free I nodes */
		if (! lsxfs_write16 (fs, fs->inode[i]))
			return 0;
	}
	if (! lsxfs_write8 (fs, fs->flock))	/* lock during free list manipulation */
		return 0;
	if (! lsxfs_write8 (fs, fs->ilock))	/* lock during I list manipulation */
		return 0;
	if (! lsxfs_write8 (fs, fs->fmod))	/* super block modified flag */
		return 0;
	if (! lsxfs_write8 (fs, fs->ronly))	/* mounted read-only flag */
		return 0;
	if (! lsxfs_write32 (fs, fs->time))	/* current date of last update */
		return 0;
	fs->dirty = 0;
	return 1;
}

void lsxfs_print (lsxfs_t *fs, FILE *out)
{
	int i;

	fprintf (out, "                File: %s\n", fs->filename);
	fprintf (out, "         Volume size: %u blocks\n", fs->fsize);
	fprintf (out, "     Inode list size: %u blocks\n", fs->isize);
	fprintf (out, "   In-core free list: %u blocks", fs->nfree);
	if (verbose)
	    for (i=0; i < 100 && i < fs->nfree; ++i) {
		if (i % 10 == 0)
			fprintf (out, "\n                     ");
		fprintf (out, " %u", fs->free[i]);
	    }
	fprintf (out, "\n");

	fprintf (out, " In-core free inodes: %u inodes", fs->ninode);
	if (verbose)
	    for (i=0; i < 100 && i < fs->nfree; ++i) {
		if (i % 10 == 0)
			fprintf (out, "\n                     ");
		fprintf (out, " %u", fs->inode[i]);
	    }
	fprintf (out, "\n");
	if (verbose) {
		fprintf (out, "      Free list lock: %u\n", fs->flock);
		fprintf (out, "     Inode list lock: %u\n", fs->ilock);
		fprintf (out, "Super block modified: %u\n", fs->fmod);
		fprintf (out, "   Mounted read-only: %u\n", fs->ronly);
	}
	fprintf (out, "    Last update time: %s", ctime (&fs->time));
}

void lsxfs_close (lsxfs_t *fs)
{
	if (fs->fd < 0)
		return;

	close (fs->fd);
	fs->fd = -1;
}

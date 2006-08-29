#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "lsxfs.h"

extern int verbose;

/*
 * get name of boot load program
 * and read onto block 0
 */
int lsxfs_install_boot (lsxfs_t *fs, const char *filename,
	const char *filename2)
{
	int fd, fd2, n, n2;
	unsigned char buf [512], buf2 [256];

	fd = open (filename, 0);
	if (fd < 0)
		return 0;
	fd2 = open (filename2, 0);
	if (fd2 < 0) {
		close (fd);
		return 0;
	}

	/* Check a.out header. */
	if (read (fd, buf, 16) != 16 || ! (buf[0] == 7 && buf[1] == 1)) {
failed:		close (fd);
		close (fd2);
		return 0;
	}
	if (read (fd2, buf2, 16) != 16 || ! (buf2[0] == 7 && buf2[1] == 1))
		goto failed;

	/* Check .text+.data segment size. */
	n = buf[2] + (buf[3] << 8) + buf[4] + (buf[5] << 8);
	n2 = buf2[2] + (buf2[3] << 8) + buf2[4] + (buf2[5] << 8);
	if (n > 128 || n2 > 384 + 256 || n2 <= 384)
		goto failed;
	if (verbose)
		printf ("Boot sector size: %d + %d bytes\n", n, n2);

	if (read (fd, buf, n) != n)
		goto failed;
	if (read (fd2, buf+128, 384) != 384)
		goto failed;
	if (read (fd2, buf2, n2-384) != n2-384)
		goto failed;
	close (fd);
	close (fd2);

	if (! lsxfs_seek (fs, 0))
		return 0;
	if (! lsxfs_write (fs, buf, 512))
		return 0;
	if (! lsxfs_seek (fs, 256000))
		return 0;
	if (! lsxfs_write (fs, buf2, 256))
		return 0;
	return 1;
}

static int free_block (lsxfs_t *fs, unsigned int bno)
{
	int i;
	unsigned short buf [256];

	if (fs->nfree >= 100) {
		buf[0] = lsx_short (fs->nfree);
		for (i=0; i<100; i++)
			buf[i+1] = lsx_short (fs->free[i]);
		if (! lsxfs_write_block (fs, bno, (char*) buf))
			return 0;
		fs->nfree = 0;
	}
	fs->free [fs->nfree] = bno;
	fs->nfree++;
	return 1;
}

static int build_inode_list (lsxfs_t *fs)
{
	lsxfs_inode_t inode;
	unsigned int inum, total_inodes;

	total_inodes = fs->isize * 16;
	for (inum = 1; inum <= total_inodes; inum++) {
		if (! lsxfs_inode_get (fs, &inode, inum))
			return 0;
		if (inode.mode == 0) {
			fs->inode [fs->ninode++] = inum;
			if (fs->ninode >= 100)
				break;
		}
	}
	return 1;
}

static unsigned int allocate_block (lsxfs_t *fs)
{
	int bno, i;
	unsigned short buf [256];

	fs->nfree--;
	bno = fs->free [fs->nfree];
	fs->free [fs->nfree] = 0;
	if (bno == 0)
		return 0;
	if (fs->nfree <= 0) {
		if (! lsxfs_read_block (fs, bno, (char*) buf))
			return 0;
		fs->nfree = lsx_short (buf[0]);
		for (i=0; i<100; i++)
			fs->free[i] = lsx_short (buf[i+1]);
	}
	return bno;
}

static int create_root_directory (lsxfs_t *fs)
{
	lsxfs_inode_t inode;
	unsigned char buf [512];
	unsigned int bno;

	memset (&inode, 0, sizeof(inode));
	inode.mode = INODE_MODE_ALLOC | INODE_MODE_FDIR | 0777;
	inode.fs = fs;
	inode.number = 1;

	/* directory - put in extra links */
	memset (buf, 0, sizeof(buf));
	buf[0] = inode.number;
	buf[1] = inode.number >> 8;
	buf[2] = '.';
	buf[16] = inode.number;
	buf[17] = inode.number >> 8;
	buf[18] = '.';
	buf[19] = '.';
	inode.nlink = 2;
	inode.size = 32;

	bno = allocate_block (fs);
	if (! lsxfs_write_block (fs, bno, buf))
		return 0;
	inode.addr[0] = bno;

	time (&inode.atime);
	time (&inode.mtime);

	if (! lsxfs_inode_save (&inode))
		return 0;
	return 1;
}

int lsxfs_create (lsxfs_t *fs, const char *filename, unsigned long bytes)
{
	int n;
	unsigned char buf [512];

	memset (fs, 0, sizeof (*fs));
	fs->filename = filename;
	fs->seek = 0;

	fs->fd = open (fs->filename, O_CREAT | O_TRUNC | O_RDWR, 0666);
	if (fs->fd < 0)
		return 0;
	fs->writable = 1;

	/* get total disk size
	 * and inode block size */
	fs->fsize = bytes / 512;
	fs->isize = (fs->fsize / 6 + 15) / 16;
	if (fs->isize < 1)
		return 0;

	/* build a list of free blocks */
	free_block (fs, 0);
	for (n = fs->fsize - 1; n >= fs->isize + 2; n--)
		if (! free_block (fs, n))
			return 0;

	/* initialize inodes */
	memset (buf, 0, 512);
	if (! lsxfs_seek (fs, 1024))
		return 0;
	for (n=0; n < fs->isize; n++)
		if (! lsxfs_write (fs, buf, 512))
			return 0;

	/* root directory */
	if (! create_root_directory (fs))
		return 0;

	/* build a list of free inodes */
	if (! build_inode_list (fs))
		return 0;

	/* write out super block */
	time (&fs->time);
	return lsxfs_sync (fs, 1);
}

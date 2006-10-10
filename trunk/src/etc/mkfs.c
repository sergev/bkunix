/*
 * Make a new file system.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/fs.h>
#include <sys/stat.h>

#ifndef S_ALLOC
#define S_ALLOC 0100000
#endif

char *boot = "/etc/boot/rxboot";
char *boot2 = "/etc/boot/rxboot2";

struct filesys fs;
unsigned char buf [512], buf2 [256];

int
setboot ()
{
	int fd, fd2, n, n2;

	fd = open (boot, 0);
	if (fd < 0) {
		printf ("%s: no such file\n", boot);
		return 0;
	}
	fd2 = open (boot2, 0);
	if (fd2 < 0) {
		printf ("%s: no such file\n", boot2);
		close (fd);
		return 0;
	}

	/* Check a.out header. */
	if (read (fd, buf, 16) != 16 || ! (buf[0] == 7 && buf[1] == 1)) {
		printf ("%s: invalid format\n", boot);
failed:		close (fd);
		close (fd2);
		return 0;
	}
	if (read (fd2, buf2, 16) != 16 || ! (buf2[0] == 7 && buf2[1] == 1)) {
		printf ("%s: invalid format\n", boot2);
		goto failed;
	}

	/* Check .text+.data segment size. */
	n = buf[2] + (buf[3] << 8) + buf[4] + (buf[5] << 8);
	n2 = buf2[2] + (buf2[3] << 8) + buf2[4] + (buf2[5] << 8);
	if (n > 128 || n2 > 384 + 256 || n2 <= 384) {
		printf ("mkfs: incorrect boot size\n");
		goto failed;
	}
	/*if (verbose)
		printf ("Boot sector size: %d + %d bytes\n", n, n2);*/

	if (read (fd, buf, n) != n) {
		printf ("%s: read error\n", boot);
		goto failed;
	}
	if (read (fd2, buf+128, 384) != 384 ||
	    read (fd2, buf2, n2-384) != n2-384) {
		printf ("%s: read error\n", boot2);
		goto failed;
	}
	close (fd);
	close (fd2);

	if (! fs_seek (&fs, 0L)) {
		printf ("mkfs: seek error\n");
		return 0;
	}
	if (! fs_write (&fs, buf, 512)) {
		printf ("mkfs: write error\n");
		return 0;
	}
	if (! fs_seek (&fs, 256000L)) {
		printf ("mkfs: seek error\n");
		return 0;
	}
	if (! fs_write (&fs, buf2, 256)) {
		printf ("mkfs: write error\n");
		return 0;
	}
	return 1;
}

int
mkilist ()
{
	struct inode inode;
	unsigned int inum, total_inodes;

	total_inodes = fs.isize * 16;
	for (inum = 1; inum <= total_inodes; inum++) {
		if (! fs_iget (&fs, &inode, inum))
			return 0;
		if (inode.mode == 0) {
			fs.inode [fs.ninode++] = inum;
			if (fs.ninode >= 100)
				break;
		}
	}
	return 1;
}

int
mkroot ()
{
	struct inode inode;
	unsigned int bno;

	memset ((char*) &inode, 0, sizeof(inode));
	inode.mode = S_ALLOC | S_IFDIR | 0777;
	inode.fs = &fs;
	inode.number = 1;

	/* directory - put in extra links */
	memset ((char*) buf, 0, sizeof(buf));
	buf[0] = inode.number;
	buf[1] = inode.number >> 8;
	buf[2] = '.';
	buf[16] = inode.number;
	buf[17] = inode.number >> 8;
	buf[18] = '.';
	buf[19] = '.';
	inode.nlink = 2;
	inode.size = 32;

	if (! fs_balloc (&fs, &bno))
		return 0;
	if (! fs_bwrite (&fs, bno, buf))
		return 0;
	inode.addr[0] = bno;

	time (&inode.atime);
	time (&inode.mtime);

	if (! fs_isave (&inode, 1))
		return 0;
	return 1;
}

int
mkfs (filename, nblocks)
	char *filename;
	int nblocks;
{
	int n;

	memset ((char*) &fs, 0, sizeof (fs));
	fs.filename = filename;
	fs.seek = 0;

	fs.fd = open (fs.filename, O_RDWR);
	if (fs.fd < 0) {
		printf ("%s: no such device\n", fs.filename);
		return 0;
	}
	fs.writable = 1;

	/* get total disk size
	 * and inode block size */
	fs.fsize = nblocks;
	fs.isize = (fs.fsize / 6 + 15) / 16;
	if (fs.isize < 1) {
		printf ("mkfs: too small filesystem size\n");
		return 0;
	}

	/* build a list of free blocks */
	fs_bfree (&fs, 0);
	for (n = fs.fsize - 1; n >= fs.isize + 2; n--)
		if (! fs_bfree (&fs, n)) {
			printf ("mkfs: cannot build list of free blocks\n");
			return 0;
		}
/*printf ("**done freeing blocks\n");*/

	/* initialize inodes */
	memset ((char*) buf, 0, 512);
	if (! fs_seek (&fs, 1024L))
		return 0;
/*printf ("**seek %ld\n", fs.seek);*/
	for (n=0; n < fs.isize; n++) {
		if (! fs_write (&fs, buf, 512)) {
			printf ("mkfs: error clearing inodes\n");
			return 0;
		}
/*printf ("**seek %ld\n", fs.seek);*/
	}

	/* root directory */
	if (! mkroot ()) {
		printf ("mkfs: error creating root directory\n");
		return 0;
	}

	/* build a list of free inodes */
	if (! mkilist ()) {
		printf ("mkfs: error building list of inodes\n");
		return 0;
	}

	/* write out super block */
	if (! fs_sync (&fs, 1)) {
		printf ("mkfs: error writing superblock\n");
		return 0;
	}
	return 1;
}

int
main (argc, argv)
	int argc;
	char **argv;
{
	char *devname;
	int nblocks;

	if (argc != 3) {
usage:		printf ("Usage: mkfs device blocks\n");
		return 1;
	}
	devname = argv[1];
	nblocks = atoi(argv[2]);
/*printf ("mkfs %s %d\n", devname, nblocks);*/
	if (nblocks <= 0)
		goto usage;

	/* Create new filesystem. */
	if (! mkfs (devname, nblocks))
		return 1;
	printf ("Created filesystem %s - %d blocks\n", devname, nblocks);

	/*if (! setboot ())
		return 1;*/

	fs_close (&fs);
	return 0;
}

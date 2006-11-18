/*
 * Display free disk space.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <mtab.h>
#include <sys/stat.h>
#include <sys/fs.h>

int iflag;
int timesthrough;
struct filesys sblock;
struct stat root;
int buf[256];

void
bread(fd, bno, data)
	char *data;
{
	int n;

	lseek(fd, (unsigned long) bno << 9, 0);
	n = read(fd, data, 512);
	if (n != 512) {
		printf("df: block %d: read error\n", bno);
		exit(1);
	}
}

/*
 * Count a number of used inodes.
 */
int
niused(fd)
{
	int n, *p, iused;

	iused = 0;
	for (n=0; n<sblock.isize; ++n) {
		bread(fd, n + 2, buf);
		for (p=buf; p<buf+256; p+=FS_INODE_SIZE/2) {
			if (*p & S_ALLOC)
				++iused;
		}
	}
	return iused;
}

/*
 * Fetch a block from free list.
 */
int
alloc(fd)
{
	int b, i;

	i = --sblock.nfree;
	if (i < 0 || i >= 100) {
		printf("df: bad free count\n");
		return 0;
	}
	b = sblock.free[i];
	if (b == 0)
		return 0;
	if (b < sblock.isize + 2 || b >= sblock.fsize) {
		printf("df: bad free block %d\n", b);
		return 0;
	}
	if (sblock.nfree <= 0) {
		bread(fd, b, buf);
		sblock.nfree = buf[0];
		for (i=0; i<100; i++)
			sblock.free[i] = buf[i+1];
	}
	return b;
}

void
prdir(dev, ino)
{
	DIR *dir;
	register struct dirent *d;

	if (dev == root.st_dev) {
		if (ino <= 1) {
			printf(ino==1 ? "/" : "--");
			return;
		}
		/* Scan root to find a possible mount point. */
		dir = opendir("/");
		if (dir) {
			for (;;) {
				d = readdir(dir);
				if (! d)
					break;
				if (d->d_ino == ino) {
					printf("/%.14s", d->d_name);
					return;
				}
			}
		}
	}
	printf("(inode %d dev %d/%d)", ino, dev >> 8, dev & 0377);
}

void
dfree(fs_dev, on_dev, on_ino)
{
	char file[10];
	int fd, nfree, avail, used, inodes, iused = 0;

	strcpy (file, "/dev/fd0");
	file[7] += fs_dev;
	fd = open(file, 0);
	if (fd < 0) {
		printf("df: `%s': cannot open\n", file);
		return;
	}
	sync();
	bread(fd, 1, buf);
	memcpy (&sblock, buf, sizeof(sblock));
	if (sblock.fsize < 6 || sblock.isize < 1 ||
	    sblock.isize > 4096 || sblock.isize > sblock.fsize / 4 ||
	    sblock.nfree > sblock.fsize) {
		printf("df: `%s': bad filesystem\n", file);
		close(fd);
		return;
	}
	nfree = 0;
	while (alloc(fd))
		nfree++;
	if (iflag)
		iused =	niused(fd);
	close(fd);
	avail = sblock.fsize - sblock.isize;
	used = avail - nfree;

	if (++timesthrough == 1) {
		printf("Filesystem  512-blks  Used  Avail Capacity");
		if (iflag)
			printf(" iused  ifree %%iused");
		printf("  Mounted on\n");
	}
	printf("%-12s", file);
	printf(" %7d %5d %6d", sblock.fsize, used, nfree);
	printf(" %5d%%", (int) ((used * 100L + avail/2) / avail));
	if (iflag) {
		inodes = sblock.isize * (512 / FS_INODE_SIZE);
		printf(" %6u %6u %5d%%   ", iused, inodes - iused,
			(int) ((iused * 100L + inodes/2) / inodes));
	} else
		printf("    ");

	prdir (on_dev, on_ino);
	printf("\n");
}

int
main(argc, argv)
	char **argv;
{
	char *arg;
	struct stat st;
	int n, mt, fs_dev, fs_ronly, on_dev, on_ino;

	stat ("/", &root);
	mt = openmtab();
	if (! mt) {
		printf("df: cannot open mount table\n");
		return 1;
	}
	n = 0;
	while (--argc > 0) {
		arg = *++argv;
		if (arg[0] == '-') {
			if (arg[1] == 'i') {
				iflag++;
				continue;
			}
		}
		n++;
		if (stat(arg, &st) < 0) {
			printf("df: `%s': no such file\n", arg);
			continue;
		}
		if (S_ISBLK(st.st_mode))
			st.st_dev = st.st_addr[0];
		resetmtab(mt);
		do {
			if (readmtab(mt, &fs_dev, &fs_ronly, &on_dev,
			    &on_ino) < 0) {
				fs_dev = st.st_dev;
				on_dev = 0;
				on_ino = 0;
				break;
			}
		} while (fs_dev != st.st_dev);

		dfree(fs_dev, on_dev, on_ino);
	}
	if (n == 0) {
		while (readmtab(mt, &fs_dev, &fs_ronly,
		    &on_dev, &on_ino) >= 0) {
			dfree(fs_dev, on_dev, on_ino);
		}
	}
	return 0;
}

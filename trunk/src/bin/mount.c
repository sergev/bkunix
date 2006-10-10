/*
 * Mount file systems.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <mtab.h>

void
prdir(dev, ino)
{
	DIR *dir;
	register struct dirent *d;

	if (dev == 0) {
		if (ino == 1) {
			printf("/");
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
	printf("inode %d dev %d/%d", ino, dev >> 8, dev & 0377);
}

void
printmtab()
{
	int mt, fs_dev, ro, on_dev, on_ino;

	mt = openmtab();
	if (! mt) {
		printf("mount: cannot open mount table\n");
		return;
	}
	while (readmtab(mt, &fs_dev, &ro, &on_dev, &on_ino) >= 0) {
		printf("/dev/fd%d on ", fs_dev);
		prdir(on_dev, on_ino);
		printf(" (%s)\n", ro ? "readonly" : "read-write");
	}
}

int
main(argc, argv)
	char **argv;
{
	register int ro;

	if (argc == 1) {
		printmtab();
		return 0;
	}
	if (argc < 3) {
		printf("Usage: mount [-r] special node\n");
		return 1;
	}
	ro = 0;
	if (argc > 3)
		ro++;
	if (mount(argv[1], argv[2], ro) < 0) {
		printf("%s: failed mount on %s %s\n", argv[1], argv[2],
			ro ? "readonly" : "read-write");
		return 1;
	}
	return 0;
}

/*
 * Print working (current) directory
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

char path[512];
int off	= -1;

void
cat(name)
	char *name;
{
	register int i, j;

	i = -1;
	while (name[++i] != 0);
	if ((off+i+2) > 511) {
		write(2, "pwd: too long path\n", 19);
		exit(1);
	}
	for (j=off+1; j>=0; --j)
		path[j+i+1] = path[j];
	off=i+off+1;
	path[i] = '/';
	for (--i; i>=0; --i)
		path[i] = name[i];
}

char * ckroot(dev, ino)
int dev, ino;
{
	DIR * dir;
	struct stat st;
	register struct dirent * d;
	chdir("/");
	dir = opendir("/");
	do {
		d = readdir(dir);
		if (! d) {
			return "[mount point]";
		}
		stat(d->d_name, &st);
	} while (st.st_dev != dev || st.st_ino != ino);
	closedir(dir);
	return d->d_name;
}

int
main()
{
	int rdev, rino;
	DIR *dir;
	struct stat dot, dotdot;
	register struct dirent *d;

	stat("/", &dot);
	rdev = dot.st_dev;
	rino = dot.st_ino;
/*printf ("/ dev=%d ino=%d\n", rdev, (int) rino);*/
	for (;;) {
		stat(".", &dot);
/*printf (". dev=%d ino=%d\n", (int) dot.st_dev, (int) dot.st_ino);*/
		if (dot.st_ino == rino && dot.st_dev == rdev)
			break;
		dir = opendir("..");
		if (! dir) {
			write(2, "pwd: cannot open ..\n", 20);
			return 1;
		}
		stat("..", &dotdot);
/*printf (".. dev=%d ino=%d\n", (int) dotdot.st_dev, (int) dotdot.st_ino);*/
		chdir("..");
		if (dot.st_dev == dotdot.st_dev) {
			if (dot.st_ino == dotdot.st_ino) {
				cat(ckroot(dot.st_dev, dot.st_ino));
				break;
			}
			do {
				d = readdir(dir);
				if (! d) {
					write(2, "read error in ..\n", 17);
					return 1;
				}
			} while (d->d_ino != dot.st_ino);
		} else {
			do {
				d = readdir(dir);
				if (! d) {
					write(2, "read error in ..\n", 17);
					return 1;
				}
				stat(d->d_name, &dotdot);
/*printf ("%.14s dev=%d ino=%d\n", d->d_name, (int) dotdot.st_dev, (int) dotdot.st_ino);*/
			} while (dotdot.st_ino != dot.st_ino ||
				dotdot.st_dev != dot.st_dev);
		}
		closedir(dir);
		cat(d->d_name);
/*path[off] = 0; printf ("pwd %s\n", path);*/
	}
	write(1, "/", 1);
	if (off < 0)
		off = 0;
	path[off] = '\n';
	write(1, path, off + 1);
	return 0;
}

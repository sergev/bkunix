/*
 * Concatenate files.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

char buf[512];

int
main(argc, argv)
	char **argv;
{
	int fflg = 0;
	register int fi, n;
	int dev, ino;
	struct stat statb;

	for (; argc>1 && argv[1][0]=='-'; argc--, argv++) {
		switch (argv[1][1]) {
		case 0:
			break;
		case 'u':
			continue;
		}
		break;
	}
	fstat(1, &statb);
	statb.st_mode &= S_IFMT;
	ino = -1;
	if (statb.st_mode != S_IFCHR && statb.st_mode != S_IFBLK) {
		dev = statb.st_dev;
		ino = statb.st_ino;
	}
	if (argc < 2) {
		argc = 2;
		fflg++;
	}
	while (--argc > 0) {
		if (fflg || ((*++argv)[0] == '-' && (*argv)[1] == '\0'))
			fi = 0;
		else {
			fi = open(*argv, 0);
			if (fi < 0) {
				printf("cat: can't open %s\n", *argv);
				continue;
			}
		}
		fstat(fi, &statb);
		if (statb.st_dev == dev && statb.st_ino == ino) {
			printf("cat: input %s is output\n",
				fflg ? "-" : *argv);
			close(fi);
			continue;
		}
		while ((n = read(fi, buf, 512)) > 0)
			write(1, buf, n);
		if (fi != 0)
			close(fi);
	}
	return 0;
}

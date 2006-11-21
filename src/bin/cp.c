/*
 * cp oldfile newfile
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

char buf[512];
struct stat sold, snew;

int
main(argc, argv)
	char **argv;
{
	char *srcname, *dstname;
	int fold, fnew, n;
	register char *s;
	int mode;

	if (argc != 3) {
		write(1, "Usage: cp oldfile newfile\n", 26);
		exit(1);
	}
	srcname = argv[1];
	dstname = argv[2];

	fold = open(srcname, 0);
	if (fold < 0) {
		write(1, "Cannot open old file.\n", 22);
		exit(1);
	}
	fstat(fold, &sold);
	mode = sold.st_mode;

	/* is target a directory? */
	if (stat(dstname, &snew) >= 0 && S_ISDIR(snew.st_mode)) {
		strcpy (buf, dstname);
		s = strrchr (srcname, '/');
		if (s)
			strcat (buf, s);
		else {
			strcat (buf, "/");
			strcat (buf, srcname);
		}
		dstname = buf;
	}
	if (stat(dstname, &snew) >= 0) {
		if (sold.st_dev==snew.st_dev && sold.st_ino==snew.st_ino) {
			write(1, "Copying file to itself.\n", 24);
			exit(1);
		}
	}
	fnew = creat(dstname, mode);
	if (fnew < 0) {
		write(1, "Can't create new file.\n", 23);
		exit(1);
	}
	while ((n = read(fold, buf, 512)) > 0) {
		if (write(fnew, buf, n) != n){
			write(1, "Write error.\n", 13);
			exit(1);
		}
	}
	if (n < 0) {
		write(1, "Read error\n", 11);
		exit(1);
	}
	return 0;
}

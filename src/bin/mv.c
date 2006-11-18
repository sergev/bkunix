/*
 * Move files:
 *	mv [-d] file1 file2
 *
 * unlink file2
 * link file1 file2
 * unlink file1
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

struct stat st;
struct stat st2;

char strbuf[70];

int
main(argc, argv)
	int argc;
	char **argv;
{
	char **argp;
	char *argp1, *argp2, *argp3, *argp4;
	char *p, *p1, *p2;
	char place[100];
	int i;
	int status;
	int b;

	argp = argv;
	/*
	 * check for correct number
	 * of arguments
	 */
	if (argc != 3) {
		write(1, "Usage: mv name1 name2\n", 22);
		exit(1);
	}
	/*
	 * is there anything to do?
	 */
	argp3 = argp[1];
	argp4 = argp[2];
	if (stat(argp[1], &st) < 0) {
		write(1, "Source file non-existent\n", 25);
		exit(1);
	}
	/*
	 * yes, there is a source.
	 * check whether file or directory
	 */
	if (S_ISDIR(st.st_mode)) {
		/*
		 * The source is a directory, so
		 * we do lots of checking and
		 * messing around so as not to get
		 * into trouble.  This patch of
		 * code contains administrative
		 * policies rather than system
		 * restrictions.
		 */
		if (stat(argp[2], &st) >= 0) {
			write(1, "Directory target exists.\n", 25);
			exit(1);
		}
		argp1 = argp[1];
		argp2 = argp[2];
		while (*argp1 == *argp2) {
			argp1++;
			if (*argp2++ == 0) {
				write(1, "???\n", 4);
				exit(1);
			}
		}
		while (*argp1)
			if (*argp1++ == '/') {
				write(1, "Directory rename only\n", 22);
				exit(1);
			}
		while (*argp2)
			if (*argp2++ == '/') {
				write(1, "Directory rename only\n", 22);
				exit(1);
			}
		if (*--argp1 == '.') {
			write(1, "values of B will give rise to dom!\n", 37);
			exit(1);
		}
	} else {
		/*
		 * the source is a file.
		 */
		setuid(getuid());
		if (stat(argp4, &st2) >= 0) {
			if (S_ISDIR(st2.st_mode)) {
				argp2 = strbuf;
				while ((*argp2++ = *argp4++));
				argp2[-1] = '/';
				argp4 = argp[1];
				argp1 = argp[1];
				while (*argp4)
					if (*argp4++ == '/')
						argp1 = argp4;
				while ((*argp2++ = *argp1++));
				argp4 = strbuf;
			}
			if (stat(argp4, &st2) >= 0) {
				if (st.st_dev == st2.st_dev &&
				    st.st_ino == st.st_ino) {
					write(1, "Files are identical.\n", 21);
					exit(1);
				}
				if ((getuid() & 0377) == st2.st_uid)
					b = 0200;
				else
					b = 02;
				if ((st2.st_mode & b) == 0 && isatty(0)) {
					printf("%s: %o mode ", argp4,
						st2.st_mode & 07777);
					i = b = getchar();
					while (b != '\n' && b != '\0')
						b = getchar();
					if (i != 'y')
						exit(1);
				}
				if (unlink(argp4) < 0) {
					write(1, "Cannot remove target file.\n", 27);
					exit(1);
				}
			}
		}
	}
	if (link(argp3, argp4) < 0) {
		i = fork();
		if (i == -1) {
			write(1, "Try again.\n", 11);
			exit(1);
		}
		if (i) {
			while (wait(&status) != i);
		} else {
			p = place;
			p1 = p;
			while ((*p++ = *argp3++));
			p2 = p;
			while ((*p++ = *argp4++));
			execl("/bin/cp", "cp", p1, p2, 0);
			write(1, "no cp\n", 6);
			exit(1);
		}
		if ((status & 0377) != 0) {
			write(1, "?\n", 2);
			exit(1);
		}
		if (status != 0)
			exit(1);
	}
	if (unlink(argp3) < 0) {
		write(1, "Cannot unlink source file.\n", 27);
		exit(1);
	}
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int
getchar()
{
	char c;

	if (read(0, &c, 1) != 1)
		return 0;
	return c;
}

void
rm(arg, fflg, rflg)
	char *arg;
{
	char *p;
	struct stat buf;
	int i, b, status;

	if (stat(arg, &buf)) {
		printf("%s: non existent\n", arg);
		return;
	}
	if (S_ISDIR(buf.st_mode)) {
		if (rflg) {
			i = fork();
			if (i == -1) {
				printf("%s: try again\n", arg);
				return;
			}
			if (i) {
				while (wait(&status) != i);
				return;
			}
			if (chdir(arg)) {
				printf("%s: cannot chdir\n", arg);
				exit(1);
			}
			p = 0;
			execl("/etc/glob", "glob", "rm", "-r",
				fflg? "-f": "*", fflg? "*": p, 0);
			printf("%s: no glob\n", arg);
			exit(1);
		}
		printf("%s: directory\n", arg);
		return;
	}

	if (! fflg) {
		if ((getuid() & 0377) == buf.st_uid)
			b = 0200;
		else
			b = 2;
		if ((buf.st_mode & b) == 0 && isatty(0)) {
			printf("%s: %o mode ", arg, buf.st_mode);
			i = b = getchar();
			i = b;
			while (b != '\n' && b != '\0')
				b = getchar();
			if (i != 'y')
				return;
		}
	}
	if (unlink(arg))
		printf("%s: not removed\n", arg);
}

int
main(argc, argv)
	char **argv;
{
	char *arg;
	int fflg, rflg;

	fflg = 0;
	rflg = 0;
	while (--argc > 0) {
		arg = *++argv;
		if (arg[0] == '-') {
			if (arg[1] == 'f') {
				fflg++;
				continue;
			}
			if (arg[1] == 'r') {
				rflg++;
				continue;
			}
		}
		rm(arg, fflg, rflg);
	}
	return 0;
}

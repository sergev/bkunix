/*
 * make directory
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

int
makedir(d)
	char *d;
{
	char pname[128], dname[128];
	struct stat st;
	register int i, slash = 0;

	pname[0] = '\0';
	for (i = 0; d[i]; ++i)
		if(d[i] == '/')
			slash = i + 1;
	if (slash)
		strncpy(pname, d, slash);
	strcpy(pname+slash, ".");
	if (stat(pname, &st) < 0) {
		printf("mkdir: cannot access %s\n", pname);
		return 1;
	}
	if (mknod(d, 040777, 0) < 0) {
		printf("mkdir: cannot make directory %s\n", d);
		return 1;
	}
	chown(d, getuid(), getgid());
	strcpy(dname, d);
	strcat(dname, "/.");
	if (link(d, dname) < 0) {
		printf("mkdir: cannot link %s\n", dname);
		unlink(d);
		return 1;
	}
	strcat(dname, ".");
	if (link(pname, dname) < 0) {
		printf("mkdir: cannot link %s\n", dname);
		dname[strlen(dname)] = '\0';
		unlink(dname);
		unlink(d);
		return 1;
	}
	return 0;
}

int
main(argc, argv)
	char *argv[];
{
	int nerrors;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (argc < 2) {
		printf("Usage: mkdir dirname...\n");
		return 1;
	}
	nerrors = 0;
	while (--argc)
		nerrors += makedir(*++argv);
	return (nerrors != 0);
}

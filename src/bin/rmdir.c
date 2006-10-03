/*
 * Remove directory
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int
removedir(d)
	char *d;
{
	int	fd;
	char	*np, name[500];
	struct	stat st, cst;
	struct	dirent dir;

	strcpy(name, d);
	np = strrchr(name, '/');
	if (! np)
		np = name;
	if (stat(name, &st) < 0) {
		printf("rmdir: %s non-existent\n", name);
		return 1;
	}
	if (stat("", &cst) < 0) {
		printf("rmdir: cannot stat \"\"");
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFDIR) {
		printf("rmdir: %s not a directory\n", name);
		return 1;
	}
	if (st.st_ino==cst.st_ino && st.st_dev==cst.st_dev) {
		printf("rmdir: cannot remove current directory\n");
		return 1;
	}
	if ((fd = open(name,0)) < 0) {
		printf("rmdir: %s unreadable\n", name);
		return 1;
	}
	while (read(fd, (char *)&dir, sizeof dir) == sizeof dir) {
		if (dir.d_ino == 0) continue;
		if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
			continue;
		printf("rmdir: %s not empty\n", name);
		close(fd);
		return 1;
	}
	close(fd);
	if (!strcmp(np, ".") || !strcmp(np, "..")) {
		printf("rmdir: cannot remove . or ..\n");
		return 1;
	}
	strcat(name, "/..");
	if (stat(name, &st) >= 0) {
		unlink(name);	/* unlink name/.. */
	}
	name[strlen(name)-1] = '\0';
	unlink(name);	/* unlink name/.  */
	name[strlen(name)-2] = '\0';
	if (unlink(name) < 0) {
		printf("rmdir: %s not removed\n", name);
		return 1;
	}
	return 0;
}

int
main(argc,argv)
	int argc;
	char **argv;
{
	int nerrors;

	if (argc < 2) {
		printf("Usage: rmdir dirname...\n");
		return 1;
	}
	nerrors = 0;
	while (--argc)
		nerrors += removedir(*++argv);
	return (nerrors != 0);
}

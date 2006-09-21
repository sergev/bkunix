#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "a.out.h"

char	tname[] = "/tmp/sXXXXXX";
int	tf;

int
copy(name, fr, to, size)
	char *name;
	long size;
{
	register int s, n;
	char buf[512];

	while (size != 0) {
		s = 512;
		if (size < 512)
			s = size;
		n = read(fr, buf, s);
		if (n != s) {
			printf("%s unexpected eof\n", name);
			return(1);
		}
		n = write(to, buf, s);
		if (n != s) {
			printf("%s unexpected write eof\n", name);
			return(1);
		}
		size -= s;
	}
	return(0);
}

int
strip(name)
	char *name;
{
	register int f;
	long size;
	int status;
	struct exec head;

	status = 0;
	f = open(name, 0);
	if (f < 0) {
		printf("cannot open %s\n", name);
		status = 1;
		goto out;
	}
	read(f, (char*) &head, sizeof(head));
	if (N_BADMAG(head)) {
		printf("%s not in a.out format\n", name);
		status = 1;
		goto out;
	}
	if (head.a_syms == 0 && (head.a_flag & A_NRELFLG)) {
		printf("%s already stripped\n", name);
		goto out;
	}
	size = (long)head.a_text + head.a_data;
	head.a_syms = 0;
	head.a_flag |= A_NRELFLG;

	lseek(tf, (long) 0, 0);
	write(tf, (char*) &head, sizeof(head));
	if (copy(name, f, tf, size) != 0) {
		status = 1;
		goto out;
	}
	size += sizeof(head);
	close(f);
	f = creat(name, 0666);
	if (f < 0) {
		printf("%s cannot recreate\n", name);
		status = 1;
		goto out;
	}
	lseek(tf, (long)0, 0);
	if (copy(name, tf, f, size) != 0)
		status = 2;
out:
	close(f);
	return status;
}

int
main(argc, argv)
	char **argv;
{
	register int i;
	int status;

	status = 0;
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	tf = mkstemp(tname);
	if (tf < 0) {
		printf("cannot create temp file\n");
		return 2;
	}
	for (i=1; i<argc; i++) {
		status = strip(argv[i]);
		if (status != 0)
			break;
	}
	close(tf);
	unlink(tname);
	return status;
}

/*
 * Discard symbols from object files.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "a.out.h"

char	tname[] = "/tmp/sXXXXXX";
int	tf;

/*
 * Read a.out header. Return 0 on error.
 */
int
readhdr(fd, hdr)
	int fd;
	register struct exec *hdr;
{
#ifdef __pdp11__
	if (read(fd, hdr, sizeof(struct exec)) != sizeof(struct exec))
		return 0;
#else
	unsigned char buf [16];

	if (read(fd, buf, 16) != 16)
		return 0;
	hdr->a_magic = buf[0] | buf[1] << 8;
	hdr->a_text = buf[2] | buf[3] << 8;
	hdr->a_data = buf[4] | buf[5] << 8;
	hdr->a_bss = buf[6] | buf[7] << 8;
	hdr->a_syms = buf[8] | buf[9] << 8;
	hdr->a_entry = buf[10] | buf[11] << 8;
	hdr->a_unused = buf[12] | buf[13] << 8;
	hdr->a_flag = buf[14] | buf[15] << 8;
#endif
	return 1;
}

/*
 * Write a.out header.
 */
void
writehdr(fd, hdr)
	int fd;
	register struct exec *hdr;
{
#ifdef __pdp11__
	write(fd, hdr, sizeof(struct exec));
#else
	unsigned char buf [16];

	buf[0] = hdr->a_magic;
        buf[1] = hdr->a_magic >> 8;
	buf[2] = hdr->a_text;
	buf[3] = hdr->a_text >> 8;
	buf[4] = hdr->a_data;
	buf[5] = hdr->a_data >> 8;
	buf[6] = hdr->a_bss;
	buf[7] = hdr->a_bss >> 8;
	buf[8] = hdr->a_syms;
	buf[9] = hdr->a_syms >> 8;
	buf[10] = hdr->a_entry;
	buf[11] = hdr->a_entry >> 8;
	buf[12] = hdr->a_unused;
	buf[13] = hdr->a_unused >> 8;
	buf[14] = hdr->a_flag;
	buf[15] = hdr->a_flag >> 8;
	write(fd, buf, 16);
#endif
}

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
	if (! readhdr(f, &head) || N_BADMAG(head)) {
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
	writehdr(tf, &head);
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

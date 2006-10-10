/*
 * size -- determine object size
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "a.out.h"

int base;

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

void
size(filename)
	char *filename;
{
	struct exec hdr;
	int f;
	unsigned sum;
	static int header_printed;

	f = open(filename, 0);
	if (f < 0) {
		printf("%s: not found\n", filename);
		return;
	}
	if (! header_printed) {
		printf("   text    data     bss     %s     hex filename\n",
			base==8 ? "oct" : "dec");
		header_printed = 1;
	}
	if (! readhdr(f, &hdr) || N_BADMAG(hdr)) {
		printf("%s: bad format\n", filename);
		close(f);
		return;
	}
	sum = hdr.a_text + hdr.a_data + hdr.a_bss;
	printf(base==16 ? "%#7x %#7x %#7x" :
		base==8 ? "%#7o %#7o %#7o" : "%7u %7u %7u",
		hdr.a_text, hdr.a_data, hdr.a_bss);
	printf(base==8 ? " %#7o" : " %7u", sum);
	printf(" %7x %s\n", sum, filename);
	close(f);
}

int
main(argc, argv)
	char **argv;
{
	register char *cp;
	int nfiles;

	base = 10;
	nfiles = 0;
	while(--argc) {
		++argv;
		if (**argv != '-') {
			size (*argv);
			++nfiles;
			continue;
		}
		for (cp = *argv+1; *cp; cp++) {
			switch (*cp) {
			case 'o':       /* octal mode */
				base = 8;
				break;
			case 'd':       /* decimal mode */
				base = 10;
				break;
			case 'x':       /* hexadecimal mode */
				base = 16;
				break;
			default:
				fprintf (stderr, "Usage: size [-odx] file...\n");
				return (1);
			}
		}
	}
	if (nfiles == 0)
		size ("a.out");
	return 0;
}

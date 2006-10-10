/*
 * relocate command--
 * reloc file [-]octal [ - ]
 *
 * relocate object or a.out file up in core
 * by the possibly negated octal number.
 *
 * if optional 3rd arg is given,
 * replace "setd" at start by "nop"
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "a.out.h"

char	tbuf[512];
char	rbuf[512];
int	fin;
int	fout;
char	*txtp;
char	*relp;
int	relloc;
int	txtloc;
int	dotdot;
int	txtsiz;
int	t1;
int	t2;
int	t4;
int	t7;
int	txtw;

void
advance()
{
	relp += 2;
	relloc += 2;
	if (relp == &rbuf[512]) {
		lseek(fin, (long) relloc, 0);
		read(fin, &rbuf[0], 512);
		relp = &rbuf[0];
	}
	txtp += 2;
	txtloc += 2;
	if (txtp >= &tbuf[512]) {
		write(fout, &tbuf[0], txtw);
		lseek(fin, (long) txtloc, 0);
		txtw = read(fin, &tbuf[0], 512);
		txtp = &tbuf[0];
	}
}

void
usage()
{
	write(1, "Usage: reloc [-d] file [-]octal\n", 32);
	exit(1);
}

int
main(argc, argv)
	char *argv[];
{
	int sign, c, dflag;
	struct exec *hdr;
	struct nlist *symp;

	dflag = 0;
	while(argv[1] && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'd':
			/* Option -d: nop out "setd" at start */
			dflag = 1;
			break;
		default:
			usage();
		}
		++argv;
		--argc;
	}
	if (argc != 3)
		usage();

	dotdot = 0;
	if (*argv[2] == '-') {
		sign = -1;
		argv[2]++;
	} else
		sign = 1;
	while (*argv[2]) {
		c = *argv[2]++ - '0';
		if (c<0 || c>7)
			usage();
		dotdot = (dotdot<<3) + c;
	}
	dotdot *= sign;
	fin = open(argv[1], 0);
	if (fin < 0) {
		write(1, "File not readable\n", 18);
		return 1;
	}
	fout = open(argv[1], 1);
	if (fout < 0) {
		write(1, "File not writable\n", 18);
		return 1;
	}
	txtw = read(fin, tbuf, 512);
	hdr = (struct exec*) tbuf;
	if (N_BADMAG(*hdr)) {
		write(1, "Bad format\n", 11);
		return 1;
	}
	if (hdr->a_flag & A_NRELFLG) {
		write (1, "No relocation bits\n", 19);
		return 1;
	}
	t1 = hdr->a_text;
	t2 = hdr->a_data;
	t4 = hdr->a_syms;
	txtloc = N_TXTOFF(*hdr);
	txtsiz = hdr->a_text + hdr->a_data;
	relloc = N_RELOFF(*hdr);
	txtsiz = (txtsiz >> 1) & 077777;
	lseek(fin, (long) relloc & ~0777, 0);
	read(fin, rbuf, 512);
	txtp = tbuf + txtloc;
	relp = &rbuf[relloc & 0777];
	if (dflag)			/* nop out "setd" at start */
		if (*(unsigned short*) txtp == 0170011)
			*(short*) txtp = 0240;
	while (txtsiz--) {
		switch (*(short*) relp & (A_RMASK | A_RPCREL)) {
		case A_RPCREL:		/* pc ref to abs */
			*(short*) txtp -= dotdot;
			break;
		case A_RTEXT:		/* ref to text */
		case A_RDATA:		/* ref to data */
		case A_RBSS:		/* ref to bss */
/*printf("%06o", *(unsigned short*) txtp);*/
			*(short*) txtp += dotdot;
/*printf(" -> %06o (dotdot = %#o)\n", *(unsigned short*) txtp, dotdot);*/
		}
		advance();
	}
	if (txtp != &tbuf[0])
		write(fout, &tbuf[0], txtw);

	txtsiz = t4;
	relloc = N_SYMOFF(*hdr);
	lseek(fin, (long) relloc & ~0777, 0);
	lseek(fout, (long) relloc & ~0777, 0);
	txtw = read(fin, tbuf, 512);
	symp = (struct nlist*) &tbuf[relloc & 0777];
	while (txtsiz >= sizeof(struct nlist)) {
		switch (symp->n_type & N_TYPE) {
		case N_TEXT:
		case N_DATA:
		case N_BSS:
		case N_FN:
			symp->n_value += dotdot;
		}
		++symp;
		if ((char*) symp >= &tbuf[512]) {
			write(fout, tbuf, txtw);
			txtw = read(fin, tbuf, 512);
			symp = (struct nlist*) ((char*) symp - 512);
		}
		txtsiz -= sizeof(struct nlist);
	}
	if (symp != (struct nlist*) &tbuf[0])
		write(fout, &tbuf[0], txtw);
	return 0;
}

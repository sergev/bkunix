/*
 * print symbol tables for object files
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "a.out.h"

int cflg;
int nflg;
int uflg;
int rflg = 1;
int gflg;
int pflg;

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
 * Read a.out symbol. Return 0 on error.
 */
int
readsym(fd, sym)
	int fd;
	register struct nlist *sym;
{
#ifdef __pdp11__
	if (read(fd, sym, sizeof(struct nlist)) != sizeof(struct nlist))
		return 0;
#else
	unsigned char buf [4];

	if (read(fd, sym->n_name, sizeof(sym->n_name)) != sizeof(sym->n_name))
		return 0;
	if (read(fd, buf, 4) != 4)
		return 0;
	sym->n_type = buf[0] | buf[1] << 8;
	sym->n_value = buf[2] | buf[3] << 8;
#endif
	return 1;
}

int
compare(p1, p2)
	struct nlist *p1, *p2;
{
	if (nflg) {
		if (p1->n_value > p2->n_value)
			return rflg;
		if (p1->n_value < p2->n_value)
			return -rflg;
	}
	return rflg * strncmp (p1->n_name, p2->n_name, sizeof(p1->n_name));
}

int
names(filename, nameflg)
	char *filename;
{
	struct exec hdr;
	struct nlist *nlp;
	int fi, n, i, j;

	fi = open(filename, 0);
	if (fi < 0) {
		printf("%s: cannot open\n", filename);
		return 1;
	}
	if (! readhdr(fi, &hdr) || N_BADMAG(hdr)) {
		printf("%s: bad format\n", filename);
		return 1;
	}
	n = hdr.a_syms / sizeof(struct nlist);
	if (n == 0) {
		printf("%s: no name list\n", filename);
		return 1;
	}
	nlp = malloc (hdr.a_syms);
	if (! nlp) {
		printf("%s: out of memory\n", filename);
		return 1;
	}
	if (lseek(fi, N_SYMOFF(hdr), 0) < 0) {
badsym:		printf("%s: cannot read symbol table\n", filename);
		return 1;
	}
	for (i=0; i<n; i++) {
		if (! readsym(fi, nlp + i))
			goto badsym;
	}
	if (pflg == 0)
		qsort(nlp, n, sizeof(struct nlist), compare);
	if (nameflg)
		printf("\n%s:\n", filename);
	for (i=0; i<n; i++, nlp++) {
		if (gflg && ! (nlp->n_type & N_EXT))
			continue;
		if (cflg) {
			if (nlp->n_name[0] != '_')
				continue;
			for (j=0; j<sizeof(nlp->n_name)-1; j++)
				nlp->n_name[j] = nlp->n_name[j+1];
			nlp->n_name[sizeof(nlp->n_name)-1] = '\0';
		}
		j = nlp->n_type & N_TYPE;
		if (j > N_BSS)
			j = N_ABS;
		if (j==0 && nlp->n_value)
			j = N_COMM;
		if (uflg && j != 0)
			continue;
		if (! uflg || nameflg) {
			if (j == 0)
				printf("       ");
			else
				printf("%06o ", nlp->n_value);
			printf("%c ", (nlp->n_type & N_EXT ?
				"UATDBC" : "uatdbc") [j]);
		}
		printf("%.8s\n", nlp->n_name);
	}
	return 0;
}

int
main(argc, argv)
	char **argv;
{
	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) {
			switch (**argv) {
			case 'n':		/* sort numerically */
				nflg++;
				continue;
			case 'c':		/* c-style names */
				cflg++;
				continue;
			case 'g':		/* globl symbols only */
				gflg++;
				continue;
			case 'u':		/* undefined symbols only */
				uflg++;
				continue;
			case 'r':		/* sort in reverse order */
				rflg = -1;
				continue;
			case 'p':		/* don't sort -- symbol table order */
				pflg ++;
				continue;
			default:
				continue;
			}
		}
		argc--;
	}
	if (argc == 0)
		return names("a.out", 0);

	if (argc == 1)
		return names(*++argv, 0);

	while (argc-- > 0)
		names(*++argv, 1);
	return 0;
}

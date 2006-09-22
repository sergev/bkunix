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
	read(fi, &hdr, sizeof(hdr));
	if (N_BADMAG(hdr)) {
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
	if (lseek(fi, N_SYMOFF(hdr), 0) < 0 ||
	    read(fi, nlp, hdr.a_syms) != hdr.a_syms) {
		printf("%s: cannot read symbol table\n", filename);
		return 1;
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

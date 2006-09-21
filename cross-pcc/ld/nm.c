/*
 *	print symbol tables for
 *      object files (СВС-Б)
 *
 *	nm [-goprun] [name ...]
*/

# include <stdio.h>
# include <ctype.h>

# ifdef CROSS
#    include "../h/a.out.h"
#    include "../h/ar.h"
# else
#    include <a.out.h>
#    include <ar.h>
# endif

# define QUANT  2048

int numsort_flg;
int undef_flg;
int revsort_flg;
int globl_flg;
int nosort_flg;
int prep_flg;
int ar_flg;
long off;
struct exec hdr;
struct ar_hdr arhdr;
FILE *fi;
char *malloc ();
char *realloc ();
long ftell ();

# define MSG(l,r) (msg ? (r) : (l))

char msg;

initmsg ()
{
	register char *p;
	extern char *getenv ();

	msg = (p = getenv ("MSG")) && *p == 'r';
}

main (argc, argv)
char ** argv;
{
	int narg;
	int compare ();

	initmsg ();
	if (--argc > 0 && argv [1] [0] == '-' && argv [1] [1] != 0) {
		argv++;
		while (* ++ * argv) switch (** argv) {
		case 'n':		/* sort numerically */
			numsort_flg++;
			continue;
		case 'g':		/* globl symbols only */
			globl_flg++;
			continue;
		case 'u':		/* undefined symbols only */
			undef_flg++;
			continue;
		case 'r':		/* sort in reverse order */
			revsort_flg++;
			continue;
		case 'p':		/* don't sort -- symbol table order */
			nosort_flg++;
			continue;
		case 'o':		/* prepend a name to each line */
			prep_flg++;
			continue;
		default:		/* oops */
			fprintf (stderr,
				MSG ("nm: unknown flag -%c\n",
					"nm: неизвестный флаг -%c\n"),
				*argv [0]);
			exit (1);
		}
		argc--;
	}
	if (argc == 0) {
		argc = 1;
		argv [1] = "a.out";
	}
	narg = argc;
	while (argc--) {
		fi = fopen (*++argv, "r");
		if (fi == NULL) {
			fprintf (stderr,
				MSG ("nm: cannot open %s\n",
					"nm: не могу открыть %s\n"),
				*argv);
			continue;
		}
		fgetint (fi, &hdr.a_magic);
		ar_flg = hdr.a_magic==ARMAG;
		if (!ar_flg && N_BADMAG(hdr)) {
			fprintf (stderr,
				MSG ("nm: %s: bad format\n",
					"nm: %s: плохой формат\n"),
				*argv);
			continue;
		}
		off = 8L;
		do {
			if (ar_flg) {
				fseek (fi, off, 0);
				if (!fgetarhdr (fi, &arhdr)) break;
				/* offset to next element */
				off = arhdr.ar_size + ftell (fi);
				if (narg > 1) printf ("\n%s:\n", *argv);
			} else fseek (fi, 0L, 0);
			fgethdr (fi, &hdr);
			nm (*argv, narg);
		} while (ar_flg);
		fclose (fi);
	}
	exit (0);
}

nm (name, narg)
char * name;
{
	struct nlist sym;                       /* current symbol */
	register struct nlist *symp = NULL;     /* sym table */
	int symplen = 0;                        /* sym table length */
	register symindex = 0;                  /* next free table entry */
	register c;
	register long n;

	n = hdr.a_const + hdr.a_text + hdr.a_data;
	if (! (hdr.a_flag & RELFLG)) n *= 2;
	fseek (fi, n, 1);
	n = hdr.a_syms;
	if (n == 0) {
		fprintf (stderr, "nm: %s: ", name);
		if (ar_flg) fprintf (stderr, "%s: ", arhdr.ar_name);
		fprintf (stderr, MSG ("no symbol table\n",
			"нет таблицы имен\n"));
		return;
	}
	for (;;) {
		c = fgetsym (fi, &sym);
		if (c == 0) {
			fprintf (stderr,
				MSG ("nm: out of memory\n",
					"nm: мало памяти\n"));
			exit (4);
		}
		if (c == 1) break;
		n -= c;
		if (n <= 0) {
			fprintf (stderr,
				MSG ("nm: bad symbol table length\n",
					"nm: неверная длина таблицы имен\n"));
			exit (3);
		}
		if (globl_flg && (sym.n_type & N_EXT) == 0) {
			free (sym.n_name);
			continue;
		}
		switch (sym.n_type & N_TYPE) {
		default:
		case N_ABS:     c = 'a';        break;
		case N_CONST:   c = 'l';        break;
		case N_TEXT:    c = 't';        break;
		case N_DATA:    c = 'd';        break;
		case N_BSS:     c = 'b';        break;
		case N_ABSS:    c = 'y';        break;
		case N_FN:      c = 'f';        break;
		case N_UNDF:    c = 'u';        break;
		case N_COMM:    c = 'c';        break;
		case N_ACOMM:   c = 'm';        break;
		}
		if (undef_flg && c!='u') {
			free (sym.n_name);
			continue;
		}
		if (sym.n_type & N_EXT) c = toupper (c);
		sym.n_type = c;
		if (symindex == symplen) {
			if (!symplen) {
				symplen = QUANT;
				symp = (struct nlist *)
					malloc (symplen *
					sizeof (struct nlist));
			} else {
				symplen += QUANT;
				symp = (struct nlist *) realloc (symp,
					symplen * sizeof (struct nlist));
			}
			if (symp == NULL) {
				fprintf (stderr,
					MSG ("nm: out of memory on %s\n",
						"nm: мало памяти на %s\n"),
					name);
				exit(2);
			}
		}
		symp [symindex++] = sym;
	}
	if (! nosort_flg)
		qsort (symp, symindex, sizeof (struct nlist), compare);
	if ((ar_flg || narg>1) && !prep_flg) {
		printf ("\n%s:", name);
		if (ar_flg) printf (" %s:\n", arhdr.ar_name);
		else printf ("\n");
	}
	for (n=0; n<symindex; n++) {
		if (prep_flg) {
			printf ("%s:\t", name);
			if (ar_flg) printf ("%s:\t", arhdr.ar_name);
		}
		c = symp[n].n_type;
		if (! undef_flg) {
			if (c == 'u' || c == 'U') printf("        ");
			else printf ("%06o  ", (long) symp[n].n_value);
			printf (" %c ", c);
		}
		printf ("%s\n", symp[n].n_name);
	}
	while (symindex) free (symp[--symindex].n_name);
	if (symplen) free ((char *) symp);
}

compare (p1, p2)
register struct nlist *p1, *p2;
{
	register rez;

	if (numsort_flg) {
		register d = p1->n_value - p2->n_value;

		if (d>0)
			rez = 1;
		else if (d==0)
			rez = 0;
		else
			rez = -1;
	} else {
		rez = strcmp (p1->n_name, p2->n_name);
	}
	if (revsort_flg)
		rez = -rez;
	return (rez);
}

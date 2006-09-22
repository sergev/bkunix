/*
 * Link editor
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include "a.out.h"
#include "ar.h"

#define	NSYM	501	/* size of symbol table */

FILE *text;
FILE *reloc;

struct	exec	filhdr;

long	liblist[256];
long	*libp = liblist;

struct	nlist	cursym;
struct	nlist	symtab[NSYM];
struct	nlist	*hshtab[NSYM+2];
struct	nlist	*symp = symtab;		/* first free symbol table slot */
struct	nlist	*p_etext;
struct	nlist	*p_edata;
struct	nlist	*p_end;

struct nlocal {
	int symno;
	struct nlist *symref;
};

struct	nlocal	local[NSYM/2];

unsigned aflag;		/* address to relocate, default absolute 0 */
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with 'L' */
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	nflag;		/* pure procedure */
int	dflag;		/* define common even with rflag */
int	iflag;		/* I/D space separated */

char	*filname;

int	tsize;
int	dsize;
int	bsize;
int	ssize;
int	nsym;

int	torigin;
int	dorigin;
int	borigin;

int	ctrel;
int	cdrel;
int	cbrel;

int	errlev;
char	tdname[] = "/tmp/laXXXXXX";
char	tsname[] = "/tmp/lbXXXXXX";
char	ttrname[] = "/tmp/lcXXXXXX";
char	tdrname[] = "/tmp/ldXXXXXX";
FILE	*toutb;
FILE	*doutb;
FILE	*troutb;
FILE	*droutb;
FILE	*soutb;

void
cleanup()
{
	unlink("l.out");
	unlink(tdname);
	unlink(tsname);
	unlink(ttrname);
	unlink(tdrname);
}

void
fatal()
{
	cleanup();
	exit(4);
}

void
error(severe, s)
	char *s;
{
	if (filname) {
		printf("%s", filname);
		if (archdr.ar_name[0])
			printf("(%.14s)", archdr.ar_name);
		printf(": ");
	}
	printf("%s\n", s);
	if (severe)
		fatal();
	errlev = 2;
}

/*
 * Find a name in symbol table.
 * Return a pointer to a (possibly empty) hash table slot.
 */
struct nlist **
lookup(name)
	char *name;
{
	int hash;
	register struct nlist **hp;
	register char *cp;

	hash = 0;
	for (cp=name; cp < name+sizeof(cursym.n_name); )
		hash = (hash << 1) + *cp++;
	hash &= 077777;
	hp = &hshtab[hash % NSYM + 2];
	while (*hp != 0) {
		if (strncmp ((*hp)->n_name, name, sizeof(cursym.n_name)) == 0)
			break;
		++hp;
		if (hp >= &hshtab[NSYM+2])
			hp = hshtab;
	}
	return hp;
}

/*
 * Add a name from cursym to symbol table.
 * Return a pointer to the symbol table slot.
 */
struct nlist *
enter()
{
	register struct nlist *sp;

	sp = symp;
	if (sp >= &symtab[NSYM])
		error(1, "Symbol table overflow");
	memset(sp->n_name, 0, sizeof(sp->n_name));
	strncpy(sp->n_name, cursym.n_name, sizeof(sp->n_name));
	sp->n_type = cursym.n_type;
	sp->n_value = cursym.n_value;
	symp++;
	return sp;
}

/*
 * Make file symbol: set cursym to the needed value.
 */
void
mkfsym(s)
	char *s;
{
	if (sflag || xflag)
		return;
	memset(cursym.n_name, 0, sizeof(cursym.n_name));
	strncpy(cursym.n_name, s, sizeof(cursym.n_name));
	cursym.n_type = N_FN;
	cursym.n_value = torigin;
}

struct nlist *
lookloc(limit, r)
	register struct nlocal *limit;
{
	register struct nlocal *clp;
	register int sn;

	sn = (r >> 4) & 07777;
	for (clp=local; clp<limit; clp++)
		if (clp->symno == sn)
			return clp->symref;
	error(1, "Local symbol botch");
	return 0;
}

/*
 * Write 16-bit value to file.
 */
void
putword (w, fd)
	unsigned int w;
	FILE *fd;
{
	putc(w, fd);
	putc(w >> 8, fd);
}

/*
 * Read 16-bit value from file.
 */
unsigned int
getword (fd)
	FILE *fd;
{
	unsigned int w;

	w = getc(fd);
	w |= getc(fd);
	return w;
}

/*
 * Create temporary file.
 */
FILE *
tcreat(name)
	char *name;
{
	int fd;

	fd = mkstemp (name);
	if (fd < 0)
		error(1, "Can't create temp");
	return fdopen(fd, "r+");
}

/*
 * Copy temporary file to output and close it.
 */
void
copy(fd)
	register FILE *fd;
{
	register int n;
	char buf[512];

	fseek(fd, 0L, 0);
	while ((n = fread(buf, 2, 256, fd)) > 0) {
		fwrite(buf, 2, n, toutb);
	}
	fclose(fd);
}

void
readhdr(loff)
	long loff;
{
	register int st, sd;

	fseek(text, loff, 0);
	fread(&filhdr, sizeof filhdr, 1, text);
	if (filhdr.a_magic != A_FMAGIC)
		error(1, "Bad format");
	st = (filhdr.a_text + 01) & ~01;
	filhdr.a_text = st;
	cdrel = - st;
	sd = (filhdr.a_data + 01) & ~01;
	cbrel = - (st + sd);
	filhdr.a_bss = (filhdr.a_bss + 01) & ~01;
}

int
getfile(cp)
	register char *cp;
{
	register int c;

	archdr.ar_name[0] = '\0';
	if (cp[0]=='-' && cp[1]=='l') {
		filname = "/lib/libxxxxxxxxxxxxxxx";
		for(c=0; cp[c+2]; c++)
			filname[c+8] = cp[c+2];
		filname[c+8] = '.';
		filname[c+9] = 'a';
		filname[c+10] = '\0';
	} else
		filname = cp;
	text = fopen(filname, "r");
	if (! text)
		error(1, "cannot open");
	c = getword(text);
	if (ferror(text))
		error(1, "Premature EOF on file %s");
	return(c == ARCMAGIC);
}

void
symreloc()
{
	switch (cursym.n_type) {
	case N_TEXT:
	case N_EXT+N_TEXT:
		cursym.n_value += ctrel;
		return;
	case N_DATA:
	case N_EXT+N_DATA:
		cursym.n_value += cdrel;
		return;
	case N_BSS:
	case N_EXT+N_BSS:
		cursym.n_value += cbrel;
		return;
	case N_EXT+N_UNDF:
		return;
	}
	if (cursym.n_type & N_EXT)
		cursym.n_type = N_EXT + N_ABS;
}

int
load1(libflg, loff)
	long loff;
{
	register struct nlist *sp, **hp, ***lp;
	struct nlist *ssymp;
	int ndef, nloc, n;

	readhdr(loff);
	ctrel = tsize;
	cdrel += dsize;
	cbrel += bsize;
	if (filhdr.a_flag & A_NRELFLG) {
		error(0, "No relocation bits");
		return(0);
	}
	ndef = 0;
	nloc = sizeof cursym;
	ssymp = symp;

	/* Use local[] as stack of symbol references. */
	lp = (struct nlist***) local;

	loff += sizeof filhdr + filhdr.a_text + filhdr.a_text +
		filhdr.a_data + filhdr.a_data;
	fseek(text, loff, 0);
	for (n=0; n<filhdr.a_syms; n+=sizeof cursym) {
		fread(&cursym, sizeof cursym, 1, text);
		if ((cursym.n_type & N_EXT) == 0) {
			if (Xflag==0 || cursym.n_name[0]!='L')
				nloc += sizeof cursym;
			continue;
		}
		symreloc();
		hp = lookup(cursym.n_name);
		sp = *hp;
		if (sp == 0) {
			*hp = enter();
			*lp++ = hp;
			continue;
		}
		if (sp->n_type != N_EXT+N_UNDF)
			continue;
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (cursym.n_value > sp->n_value)
				sp->n_value = cursym.n_value;
			continue;
		}
		if (sp->n_value != 0 && cursym.n_type == N_EXT+N_TEXT)
			continue;
		ndef++;
		sp->n_type = cursym.n_type;
		sp->n_value = cursym.n_value;
	}
	if (libflg==0 || ndef) {
		tsize += filhdr.a_text;
		dsize += filhdr.a_data;
		bsize += filhdr.a_bss;
		ssize += nloc;
		return(1);
	}
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	symp = ssymp;
	while (lp > (struct nlist***) local)
		**--lp = 0;
	return(0);
}

void
load1arg(filename)
	register char *filename;
{
	register int loff;

	if (getfile(filename)==0) {
		load1(0, 0, 0);
		fclose(text);
		return;
	}
	loff = 2;
	for (;;) {
		fseek(text, loff, 0);
		if (fread(&archdr, sizeof archdr, 1, text) != 1) {
			*libp++ = 0;
			break;
		}
		if (load1(1, loff + sizeof(archdr))) {
			*libp++ = loff;
		}
		loff += archdr.ar_size + sizeof(archdr);
	}
	fclose(text);
}

void
middle()
{
	register struct nlist *sp;
	register int t, csize;
	int nund, corigin;

	p_etext = *lookup("_etext");
	p_edata = *lookup("_edata");
	p_end = *lookup("_end");
	/*
	 * If there are any undefined symbols, save the relocation bits.
	 */
	if (rflag==0) for (sp=symtab; sp<symp; sp++) {
		if (sp->n_type==N_EXT+N_UNDF && sp->n_value==0
		 && sp!=p_end && sp!=p_edata && sp!=p_etext) {
			rflag++;
			dflag = 0;
			nflag = 0;
			iflag = 0;
			sflag = 0;
			break;
		}
	}
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (dflag || rflag==0) {
		for (sp=symtab; sp<symp; sp++)
			if (sp->n_type==N_EXT+N_UNDF && (t=sp->n_value)!=0) {
				t = (t+1) & ~01;
				sp->n_value = csize;
				sp->n_type = N_EXT+N_COMM;
				csize += t;
			}
		if (p_etext && p_etext->n_type==N_EXT+N_UNDF) {
			p_etext->n_type = N_EXT+N_TEXT;
			p_etext->n_value = tsize;
		}
		if (p_edata && p_edata->n_type==N_EXT+N_UNDF) {
			p_edata->n_type = N_EXT+N_DATA;
			p_edata->n_value = dsize;
		}
		if (p_end && p_end->n_type==N_EXT+N_UNDF) {
			p_end->n_type = N_EXT+N_BSS;
			p_end->n_value = bsize;
		}
	}
	/*
	 * Now set symbols to their final value
	 */
	if (nflag || iflag)
		tsize = (tsize + 077) & ~077;
	dorigin = tsize;
	if (nflag)
		dorigin = (tsize+017777) & ~017777;
	if (iflag)
		dorigin = 0;
	corigin = dorigin + dsize;
	borigin = corigin + csize;
	torigin += aflag;
	dorigin += aflag;
	corigin += aflag;
	borigin += aflag;

	nund = 0;
	for (sp=symtab; sp<symp; sp++) switch (sp->n_type) {
	case N_EXT+N_UNDF:
		errlev |= 01;
		if (arflag==0 && sp->n_value==0) {
			if (nund==0)
				printf("Undefined:\n");
			nund++;
			printf("%.8s\n", sp->n_name);
		}
		continue;

	case N_EXT+N_ABS:
	default:
		continue;

	case N_EXT+N_TEXT:
		sp->n_value += torigin;
		continue;

	case N_EXT+N_DATA:
		sp->n_value += dorigin;
		continue;

	case N_EXT+N_BSS:
		sp->n_value += borigin;
		continue;

	case N_EXT+N_COMM:
		sp->n_type = N_EXT+N_BSS;
		sp->n_value += corigin;
		continue;
	}
	if (sflag || xflag)
		ssize = 0;
	bsize += csize;
	nsym = ssize / (sizeof cursym);
}

void
setupout()
{
	toutb = fopen("l.out", "w");
	if (! toutb)
		error(1, "Can't create l.out");
	doutb = tcreat(tdname);
	if (sflag==0 || xflag==0)
		soutb = tcreat(tsname);
	if (rflag) {
		troutb = tcreat(ttrname);
		droutb = tcreat(tdrname);
	}
	if (nflag)
		filhdr.a_magic = A_NMAGIC;
	else if (iflag)
		filhdr.a_magic = A_IMAGIC;
	else
		filhdr.a_magic = A_FMAGIC;
	filhdr.a_text = tsize;
	filhdr.a_data = dsize;
	filhdr.a_bss = bsize;
	filhdr.a_syms = sflag ? 0 : (ssize + (sizeof cursym)*(symp-symtab));
	filhdr.a_entry = 0;
	filhdr.a_unused = 0;
	filhdr.a_flag = rflag ? 0 : A_NRELFLG;
	fwrite(&filhdr, sizeof filhdr, 1, toutb);
}

void
load2td(lp, creloc, b1, b2)
	struct nlocal *lp;
	FILE *b1, *b2;
{
	register int r, t;
	register struct nlist *sp;

	for (;;) {
		t = getword (text);
		r = getword (reloc);
		if (ferror (reloc))
			error(1, "Relocation error");
		switch (r & A_RMASK) {
		case A_RTEXT:
			t += ctrel;
			break;
		case A_RDATA:
			t += cdrel;
			break;
		case A_RBSS:
			t += cbrel;
			break;
		case A_REXT:
			sp = lookloc(lp, r);
			if (sp->n_type==N_EXT+N_UNDF) {
				r = (r&01) + ((nsym+(sp-symtab))<<4) + A_REXT;
				break;
			}
			t += sp->n_value;
			r = (r & A_RPCREL) + ((sp->n_type - (N_EXT+N_ABS))<<1);
			break;
		}
		if (r & A_RPCREL)
			t -= creloc;
		putword(t, b1);
		if (rflag)
			putword(r, b2);
	}
}

void
load2(loff)
	long loff;
{
	register struct nlist *sp;
	register struct nlocal *lp;
	register int symno, n;

	readhdr(loff);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;
	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	lp = local;
	symno = -1;
	loff += sizeof filhdr;
	fseek(text, loff + filhdr.a_text + filhdr.a_data +
		filhdr.a_text + filhdr.a_data, 0);
	for (n=0; n<filhdr.a_syms; n+=sizeof cursym) {
		symno++;
		fread(&cursym, sizeof cursym, 1, text);
		symreloc();
		if ((cursym.n_type & N_EXT) == 0) {
			if (! sflag && ! xflag && (! Xflag ||
			    cursym.n_name[0] != 'L'))
				fwrite(&cursym, sizeof cursym, 1, soutb);
			continue;
		}
		sp = *lookup(cursym.n_name);
		if (sp == 0)
			error(1, "internal error: symbol not found");
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (lp >= &local[NSYM/2])
				error(1, "Local symbol overflow");
			lp->symno = symno;
			lp->symref = sp;
			continue;
		}
		if (cursym.n_type != sp->n_type ||
		    cursym.n_value != sp->n_value) {
			printf("%.8s: ", cursym.n_name);
			error(0, "Multiply defined");
		}
	}
	fseek(text, loff, 0);
	fseek(reloc, loff + filhdr.a_text + filhdr.a_data, 0);
	load2td(lp, ctrel, toutb, troutb);

	fseek(text, loff + filhdr.a_text, 0);
	fseek(reloc, loff + filhdr.a_text + filhdr.a_text + filhdr.a_data, 0);
	load2td(lp, cdrel, doutb, droutb);

	torigin += filhdr.a_text;
	dorigin += filhdr.a_data;
	borigin += filhdr.a_bss;
}

void
load2arg(filename)
	register char *filename;
{
	register long *lp;
	char *p;

	if (getfile(filename) == 0) {
		reloc = fdopen(fileno(text), "r");
		p = strrchr (filename, '/');
		if (p)
			filename = p+1;
		mkfsym(filename);
		fwrite(&cursym, sizeof cursym, 1, soutb);
		load2(0, 0);
		fclose(text);
		return;
	}
	reloc = fdopen(fileno(text), "r");
	for (lp = libp; *lp > 0; lp++) {
		fseek(text, *lp, 0);
		fread(&archdr, sizeof archdr, 1, text);
		mkfsym(archdr.ar_name);
		fwrite(&cursym, sizeof cursym, 1, soutb);
		load2(*lp + sizeof(archdr));
	}
	libp = ++lp;
	fclose(text);
	fclose(reloc);
}

void
finishout()
{
	register int n;
	struct nlist *p;

	if (nflag || iflag) {
		n = torigin;
		while (n & 077) {
			n += 2;
			putword(0, toutb);
			if (rflag)
				putword(0, troutb);
		}
	}
	copy(doutb);
	if (rflag) {
		copy(troutb);
		copy(droutb);
	}
	if (sflag==0) {
		if (xflag==0)
			copy(soutb);
		for (p=symtab; p < symp;)
			fwrite(p, sizeof(*p), 1, toutb);
	}
	fclose(toutb);
	unlink("a.out");
	link("l.out", "a.out");
}

int
main(argc, argv)
	char **argv;
{
	register int c;
	register char *ap, **p;
	struct nlist **hp;

	if (signal(SIGINT, SIG_DFL) == SIG_DFL)
		signal(SIGINT, fatal);
	if (argc == 1)
		exit(4);
	p = argv + 1;
	for (c = 1; c<argc; c++) {
		filname = 0;
		ap = *p++;
		if (*ap == '-') switch (ap[1]) {
		case 'u':
			if (++c >= argc)
				error(1, "Bad -u");
			hp = lookup(*p);
			if (*hp == 0) {
				*hp = symp;
				memset(cursym.n_name, 0, sizeof(cursym.n_name));
				strncpy(cursym.n_name, *p, sizeof(cursym.n_name));
				cursym.n_type = N_EXT + N_UNDF;
				cursym.n_value = 0;
				enter();
			}
			p++;
			continue;
		case 'l':
			break;
		case 'x':
			xflag++;
			continue;
		case 'X':
			Xflag++;
			continue;
		case 'r':
			rflag++;
			arflag++;
			continue;
		case 's':
			sflag++;
			xflag++;
			continue;
		case 'n':
			nflag++;
			continue;
		case 'd':
			dflag++;
			continue;
		case 'i':
			iflag++;
			continue;
		case 'a':
			if (++c >= argc)
				error(1, "Bad -a");
			aflag = strtol (*p++, 0, 0);
			continue;
		}
		load1arg(ap);
	}
	middle();
	setupout();
	p = argv+1;
	libp = liblist;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (*ap == '-') switch (ap[1]) {
		case 'u':
			++c;
			++p;
		case 'l':
			break;
		default:
			continue;
		}
		load2arg(ap);
	}
	finishout();
	cleanup();
	if (errlev != 0)
		exit(errlev);
	chmod("a.out", 0777);
	return 0;
}

/*
 *  link editor
 */
#include "a.out.h"
#include "ar.h"

#define	NSYM	501
#define	NSYMPR	500

FILE *text;
FILE *reloc;

struct liblist {
	int	off;
	int	bno;
};

struct	liblist	liblist[256];
struct	liblist	*libp { &liblist[0] };

struct	exec	filhdr;

struct	nlist	cursym;
struct	nlist	symtab[NSYM];
struct	nlist	*hshtab[NSYM+2];
struct	nlist	*symp = { symtab };
struct	nlist	**local[NSYMPR];
struct	nlist	*p_etext;
struct	nlist	*p_edata;
struct	nlist	*p_end;

unsigned aflag;		/* address to relocate, default absolute 0 */
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with 'L' */
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	nflag;		/* pure procedure */
int	dflag;		/* define common even with rflag */
int	iflag;		/* I/D space separated */

int	infil;
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
int	delarg = 4;
char	tfname[] = "/tmp/lxXXXXXX";
int	toutb[259];
int	doutb[259];
int	troutb[259];
int	droutb[259];
int	soutb[259];

#if 0
load1arg(acp)
char *acp;
{
	register char *cp;
	register noff, nbno;

	cp = acp;
	if (getfile(cp)==0) {
		load1(0, 0, 0);
		return;
	}
	nbno = 0;
	noff = 1;
	for (;;) {
		dseek(&text, nbno, noff, sizeof archdr);
		if (text.size <= 0) {
			libp->bno = -1;
			libp++;
			return;
		}
		mget(&archdr, sizeof archdr);
		if (load1(1, nbno, noff + (sizeof archdr) / 2)) {
			libp->bno = nbno;
			libp->off = noff;
			libp++;
		}
		noff += ((archdr.ar_size & 0777) + sizeof(archdr) + 1) >> 1;
		nbno += (archdr.ar_size >> 9) & 0177;
		nbno += noff >> 8;
		noff &= 0377;
	}
}

load1(libflg, bno, off)
{
	register struct nlist *sp, **hp, ***cp;
	struct nlist *ssymp;
	int ndef, nloc;

	readhdr(bno, off);
	ctrel = tsize;
	cdrel += dsize;
	cbrel += bsize;
	ndef = 0;
	nloc = sizeof cursym;
	cp = local;
	ssymp = symp;
	if (filhdr.a_flag & A_NRELFLG) {
		error(0, "No relocation bits");
		return(0);
	}
	off += (sizeof filhdr)/2 + filhdr.a_text + filhdr.a_data;
	dseek(&text, bno, off, filhdr.a_syms);
	while (text.size > 0) {
		mget(&cursym, sizeof cursym);
		if ((cursym.n_type&N_EXT)==0) {
			if (Xflag==0 || cursym.n_name[0]!='L')
				nloc += sizeof cursym;
			continue;
		}
		symreloc();
		hp = lookup();
		if ((sp = *hp) == 0) {
			*hp = enter();
			*cp++ = hp;
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
	while (cp > local)
		**--cp = 0;
	return(0);
}

middle()
{
	register struct nlist *sp;
	register t, csize;
	int nund, corigin;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end = *slookup("_end");
/*
 * If there are any undefined symbols, save the relocation bits.
 */
	if (rflag==0) for (sp=symtab; sp<symp; sp++)
		if (sp->n_type==N_EXT+N_UNDF && sp->n_value==0
		 && sp!=p_end && sp!=p_edata && sp!=p_etext) {
			rflag++;
			dflag = 0;
			nflag = 0;
			iflag = 0;
			sflag = 0;
			break;
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

setupout()
{
	register char *p;
	register pid;

	if ((toutb[0] = creat("l.out", 0666)) < 0)
		error(1, "Can't create l.out");
	pid = getpid();
	for (p = &tfname[12]; p > &tfname[7];) {
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	tcreat(doutb, 'a');
	if (sflag==0 || xflag==0)
		tcreat(soutb, 'b');
	if (rflag) {
		tcreat(troutb, 'c');
		tcreat(droutb, 'd');
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
	mput(toutb, &filhdr, sizeof filhdr);
	return;
}

tcreat(buf, letter)
int *buf;
{
	tfname[6] = letter;
	*buf = creat(tfname, 0400);
	if (*buf < 0)
		error(1, "Can't create temp");
}

load2arg(acp)
char *acp;
{
	register char *cp;
	register struct liblist *lp;

	cp = acp;
	if (getfile(cp) == 0) {
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0, 0);
		return;
	}
	for (lp = libp; lp->bno != -1; lp++) {
		dseek(&text, lp->bno, lp->off, sizeof archdr);
		mget(&archdr, sizeof archdr);
		mkfsym(archdr.ar_name);
		load2(lp->bno, lp->off + (sizeof archdr) / 2);
	}
	libp = ++lp;
}

load2(bno, off)
{
	register struct nlist *sp;
	register int *lp, symno;

	readhdr(bno, off);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;
/*
 * Reread the symbol table, recording the numbering
 * of symbols for fixing external references.
 */
	lp = local;
	symno = -1;
	off += (sizeof filhdr)/2;
	dseek(&text, bno, off + filhdr.a_text + filhdr.a_data, filhdr.a_syms);
	while (text.size > 0) {
		symno++;
		mget(&cursym, sizeof cursym);
		symreloc();
		if ((cursym.n_type&N_EXT) == 0) {
			if (!sflag&&!xflag&&(!Xflag||cursym.n_name[0]!='L'))
				mput(soutb, &cursym, sizeof cursym);
			continue;
		}
		if ((sp = *lookup()) == 0)
			error(1, "internal error: symbol not found");
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (lp >= &local[NSYMPR])
				error(1, "Local symbol overflow");
			*lp++ = symno;
			*lp++ = sp;
			continue;
		}
		if (cursym.n_type!=sp->n_type || cursym.n_value!=sp->n_value) {
			printf("%.8s: ", cursym.n_name);
			error(0, "Multiply defined");
		}
	}
	dseek(&text, bno, off, filhdr.a_text);
	dseek(&reloc, bno, off + (filhdr.a_text + filhdr.a_data)/2,
		filhdr.a_text);
	load2td(lp, ctrel, toutb, troutb);
	dseek(&text, bno, off + (filhdr.a_text/2), filhdr.a_data);
	dseek(&reloc, bno, off + filhdr.a_text + (filhdr.a_data/2),
		filhdr.a_data);
	load2td(lp, cdrel, doutb, droutb);
	torigin += filhdr.a_text;
	dorigin += filhdr.a_data;
	borigin += filhdr.a_bss;
}

load2td(lp, creloc, b1, b2)
int *lp;
{
	register r, t;
	register struct nlist *sp;

	for (;;) {
	/*
	 * The pickup code is copied from "get" for speed.
	 */
		if (--text.size <= 0) {
			if (text.size < 0)
				break;
			text.size++;
			t = get(&text);
		} else if (--text.nibuf < 0) {
			text.nibuf++;
			text.size++;
			t = get(&text);
		} else
			t = *text.ptr++;
		if (--reloc.size <= 0) {
			if (reloc.size < 0)
				error(1, "Relocation error");
			reloc.size++;
			r = get(&reloc);
		} else if (--reloc.nibuf < 0) {
			reloc.nibuf++;
			reloc.size++;
			r = get(&reloc);
		} else
			r = *reloc.ptr++;
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
		putw(t, b1);
		if (rflag)
			putw(r, b2);
	}
}

finishout()
{
	register n, *p;

	if (nflag||iflag) {
		n = torigin;
		while (n&077) {
			n += 2;
			putw(0, toutb);
			if (rflag)
				putw(0, troutb);
		}
	}
	copy(doutb, 'a');
	if (rflag) {
		copy(troutb, 'c');
		copy(droutb, 'd');
	}
	if (sflag==0) {
		if (xflag==0)
			copy(soutb, 'b');
		for (p=symtab; p < symp;)
			putw(*p++, toutb);
	}
	fflush(toutb);
	close(toutb[0]);
	unlink("a.out");
	link("l.out", "a.out");
	delarg = errlev;
	delexit();
}

copy(buf, c)
int *buf;
{
	register f, *p, n;

	fflush(buf);
	close(buf[0]);
	tfname[6] = c;
	f = open(tfname, 0);
	while ((n = read(f, doutb, 512)) > 1) {
		n =>> 1;
		p = doutb;
		do
			putw(*p++, toutb);
		while (--n);
	}
	close(f);
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cp8c(s, cursym.n_name);
	cursym.n_type = 037;
	cursym.n_value = torigin;
	mput(soutb, &cursym, sizeof cursym);
}

mget(aloc, an)
int *aloc;
{
	register *loc, n;
	register *p;

	n = an;
	n =>> 1;
	loc = aloc;
	if ((text.nibuf -= n) >= 0) {
		if ((text.size -= n) > 0) {
			p = text.ptr;
			do
				*loc++ = *p++;
			while (--n);
			text.ptr = p;
			return;
		} else
			text.size += n;
	}
	text.nibuf += n;
	do {
		*loc++ = get(&text);
	} while (--n);
}

mput(buf, aloc, an)
int *aloc;
{
	register *loc;
	register n;

	loc = aloc;
	n = an>>1;
	do {
		putw(*loc++, buf);
	} while (--n);
}

dseek(asp, ab, o, s)
{
	register struct stream *sp;
	register struct page *p;
	register b;
	int n;

	sp = asp;
	b = ab + ((o>>8) & 0377);
	o &= 0377;
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			seek(infil, b, 3);
			if ((n = read(infil, p->buff, 512)>>1) < 0)
				n = 0;
			p->nibuf = n;
		} else
			error(1, "No pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	sp->ptr = p->buff + o;
	if (s != -1)
		sp->size = (s>>1) & 077777;
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

get(asp)
struct stream *asp;
{
	register struct stream *sp;

	sp = asp;
	if (--sp->nibuf < 0) {
		dseek(sp, sp->bno+1, 0, -1);
		--sp->nibuf;
	}
	if (--sp->size <= 0) {
		if (sp->size < 0)
			error(1, "Premature EOF on %s");
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = &fpage;
	}
	return(*sp->ptr++);
}

getfile(acp)
char *acp;
{
	register char *cp;
	register c;

	cp = acp;
	archdr.ar_name[0] = '\0';
	filname = cp;
	if (cp[0]=='-' && cp[1]=='l') {
		if(cp[2] == '\0')
			cp = "-la";
		filname = "/lib/libxxxxxxxxxxxxxxx";
		for(c=0; cp[c+2]; c++)
			filname[c+8] = cp[c+2];
		filname[c+8] = '.';
		filname[c+9] = 'a';
		filname[c+10] = '\0';
	}
	if ((infil = open(filname, 0)) < 0)
		error(1, "cannot open");
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = &fpage;
	fpage.nuser = 2;
	dseek(&text, 0, 0, 2);
	if (text.size <= 0)
		error(1, "Premature EOF on file %s");
	return(get(&text) == ARCMAGIC);
}

struct nlist **lookup()
{
	int i;
	register struct nlist **hp;
	register char *cp, *cp1;

	i = 0;
	for (cp=cursym.n_name; cp < &cursym.n_name[8];)
		i = (i<<1) + *cp++;
	for (hp = &hshtab[(i&077777)%NSYM+2]; *hp!=0;) {
		cp1 = (*hp)->n_name;
		for (cp=cursym.n_name; cp < &cursym.n_name[8];)
			if (*cp++ != *cp1++)
				goto no;
		break;
	    no:
		if (++hp >= &hshtab[NSYM+2])
			hp = hshtab;
	}
	return(hp);
}

struct nlist **slookup(s)
char *s;
{
	cp8c(s, cursym.n_name);
	cursym.n_type = N_EXT+N_UNDF;
	cursym.n_value = 0;
	return(lookup());
}

enter()
{
	register struct nlist *sp;

	if ((sp=symp) >= &symtab[NSYM])
		error(1, "Symbol table overflow");
	cp8c(cursym.n_name, sp->n_name);
	sp->n_type = cursym.n_type;
	sp->n_value = cursym.n_value;
	symp++;
	return(sp);
}

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
	if (cursym.n_type&N_EXT)
		cursym.n_type = N_EXT+N_ABS;
}

error(n, s)
char *s;
{
	if (filname) {
		printf("%s", filname);
		if (archdr.ar_name[0])
			printf("(%.14s)", archdr.ar_name);
		printf(": ");
	}
	printf("%s\n", s);
	if (n)
		delexit();
	errlev = 2;
}

lookloc(alp, r)
{
	register int *clp, *lp;
	register sn;

	lp = alp;
	sn = (r>>4) & 07777;
	for (clp=local; clp<lp; clp += 2)
		if (clp[0] == sn)
			return(clp[1]);
	error(1, "Local symbol botch");
}

readhdr(bno, off)
{
	register st, sd;

	dseek(&text, bno, off, sizeof filhdr);
	mget(&filhdr, sizeof filhdr);
	if (filhdr.a_magic != A_FMAGIC)
		error(1, "Bad format");
	st = (filhdr.a_text + 01) & ~01;
	filhdr.a_text = st;
	cdrel = -st;
	sd = (filhdr.a_data + 01) & ~01;
	cbrel = - (st+sd);
	filhdr.a_bss = (filhdr.a_bss + 01) & ~01;
}

cp8c(from, to)
char *from, *to;
{
	register char *f, *t, *te;

	f = from;
	t = to;
	te = t+8;
	while ((*t++ = *f++) && t<te);
	while (t<te)
		*t++ = 0;
}
#endif /*--------------------------------------------------------------------*/

delexit()
{
	register c;

	unlink("l.out");
	for (c = 'a'; c <= 'd'; c++) {
		tfname[6] = c;
		unlink(tfname);
	}
	if (! delarg)
		chmod("a.out", 0777);
	exit(delarg);
}

int
main(argc, argv)
	char **argv;
{
	register c;
	register char *ap, **p;
	struct nlist **hp;

	if ((signal(SIGINT, 1) & 01) == 0)
		signal(SIGINT, delexit);
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
			hp = slookup(*p++);
			if (*hp == 0) {
				*hp = symp;
				enter();
			}
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
		close(infil);
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
		close(infil);
	}
	finishout();
}

/*
 * C command
 * written by John F. Reiser
 * July/August 1978
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifdef FLEXNAMES
#define	NCPS	128
#else
#define	NCPS	8
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#define STATIC

#define STDIN 0
#define SALT '#'

#ifndef BUFSIZ
#define BUFSIZ 512
#endif

char *pbeg,*pbuf,*pend;
char *outp,*inp;
char *newp;
char cinit;

/* some code depends on whether characters are sign or zero extended */
/*	#if '\377' < 0		not used here, old cpp doesn't understand */
#define COFF 128

#define ALFSIZ 256	/* alphabet size */
char macbit[ALFSIZ+11];
char toktyp[ALFSIZ];
#define BLANK 1
#define IDENT 2
#define NUMBR 3

/* a superimposed code is used to reduce the number of calls to the
 * symbol table lookup routine.  (if the kth character of an identifier
 * is 'a' and there are no macro names whose kth character is 'a'
 * then the identifier cannot be a macro name, hence there is no need
 * to look in the symbol table.)  The test is based on
 * single characters and their position in the identifier.
 * It is inexpensive, it typically costs 1 indexed fetch,
 * an AND, and a jump per character of identifier, until the identifier
 * is known as a non-macro name or until the end of the identifier.
 */
#define b0 1
#define b1 2
#define b2 4
#define b3 8
#define b4 16
#define b5 32
#define b6 64
#define b7 128

#define IB 1
#define SB 2
#define NB 4
#define CB 8
#define QB 16
#define WB 32
char fastab[ALFSIZ];
char slotab[ALFSIZ];
char *ptrtab;
#define isslo		(ptrtab == (slotab + COFF))
#define isid(a)		((fastab + COFF)[(int)a] & IB)
#define isspc(a)	(ptrtab[(int)a] & SB)
#define isnum(a)	((fastab + COFF)[(int)a] & NB)
#define iscom(a)	((fastab + COFF)[(int)a] & CB)
#define isquo(a)	((fastab + COFF)[(int)a] & QB)
#define iswarn(a)	((fastab + COFF)[(int)a] & WB)

#define eob(a)		((a) >= pend)
#define bob(a)		(pbeg >= (a))

#define cputc(a,b)	if (! flslvl) putc(a, b)

char buffer[NCPS+BUFSIZ+BUFSIZ+NCPS];

#ifdef pdp11

#define SBSIZE ((unsigned)0114130)	/* PDP compiler doesn't like 39024 */
short	sbff[SBSIZE/2];
#define sbf ((char *)sbff)

#else /* !pdp11 */

#define SBSIZE 60000		/* std = 12000, wnj aug 1979 */
char	sbf[SBSIZE];

#endif /* pdp11 */

char	*savch	= sbf;

#define DROP 0xFE	/* special character not legal ASCII or EBCDIC */
#define WARN DROP
#define SAME 0
#define MAXINC 10
#define MAXFRE 14	/* max buffers of macro pushback */
#define MAXFRM 31	/* max number of formals/actuals to a macro */

static char warnc = WARN;

int mactop,fretop;
char *instack[MAXFRE],*bufstack[MAXFRE],*endbuf[MAXFRE];

int plvl;	/* parenthesis level during scan for macro actuals */
int maclin;	/* line number of macro call requiring actuals */
char *macfil;	/* file name of macro call requiring actuals */
char *macnam;	/* name of macro requiring actuals */
int maclvl;	/* # calls since last decrease in nesting level */
char *macforw;	/* pointer which must be exceeded to decrease nesting level */
int macdam;	/* offset to macforw due to buffer shifting */

STATIC	int	inctop[MAXINC];
STATIC	char	*fnames[MAXINC];
STATIC	char	*dirnams[MAXINC];	/* actual directory of #include files */
STATIC	int	fins[MAXINC];
STATIC	int	lineno[MAXINC];

STATIC	char	*dirs[10];	/* -I and <> directories */
char *strdex(), *copy(), *subst(), *trmdir();
struct symtab *stsym();
STATIC	int	fin	= STDIN;
STATIC	FILE	*fout;
STATIC	int	nd	= 1;
STATIC	int	pflag;	/* don't put out lines "# 12 foo.c" */
int	passcom;	/* don't delete comments */
STATIC	int rflag;	/* allow macro recursion */
STATIC	int mflag;	/* generate makefile dependencies */
STATIC	char *infile;	/* name of .o file to build dependencies from */
STATIC 	FILE *mout;	/* file to place dependencies on */
#define START 1
#define CONT  2
#define BACK  3
STATIC	int	ifno;
#define NPREDEF 30
STATIC	char *prespc[NPREDEF];
STATIC	char **predef = prespc;
STATIC	char *punspc[NPREDEF];
STATIC	char **prund = punspc;
STATIC	int	exfail;
struct symtab {
	char	*name;
	char	*value;
} *lastsym, *lookup(), *slookup();

#define symsiz 2000		/* std = 500, wnj aug 1979 */
STATIC	struct symtab stab[symsiz];

STATIC	struct symtab *defloc;
STATIC	struct symtab *udfloc;
STATIC	struct symtab *incloc;
STATIC	struct symtab *ifloc;
STATIC	struct symtab *elsloc;
STATIC	struct symtab *eifloc;
STATIC	struct symtab *ifdloc;
STATIC	struct symtab *ifnloc;
STATIC	struct symtab *varloc;
STATIC	struct symtab *lneloc;
STATIC	struct symtab *ulnloc;
STATIC	struct symtab *uflloc;
STATIC	int	trulvl;
STATIC	int	flslvl;

extern 	int	yyparse();

void
sayline(where)
	int where;
{
	if (mflag && where == START)
		fprintf(mout, "%s: %s\n", infile, fnames[ifno]);
	if (pflag == 0)
		fprintf(fout,"# %d \"%s\"\n", lineno[ifno], fnames[ifno]);
}

/* VARARGS1 */
void
pperror(s,x,y)
	char *s;
{
	if (fnames[ifno][0])
		fprintf(stderr, "%s: ", fnames[ifno]);
	fprintf(stderr, "%d: ", lineno[ifno]);
	fprintf(stderr, s, x, y);
	fprintf(stderr, "\n");
	++exfail;
}

void
yyerror(s,a,b)
	char *s;
{
	pperror(s, a, b);
}

void
ppwarn(s,x)
	char *s;
{
	int fail = exfail;

	exfail = -1;
	pperror(s, x);
	exfail = fail;
}

/* data structure guide
 *
 * most of the scanning takes place in the buffer:
 *
 *  (low address)                                             (high address)
 *  pbeg                           pbuf                                 pend
 *  |      <-- BUFSIZ chars -->      |         <-- BUFSIZ chars -->        |
 *  _______________________________________________________________________
 * |_______________________________________________________________________|
 *          |               |               |
 *          |<-- waiting -->|               |<-- waiting -->
 *          |    to be      |<-- current -->|    to be
 *          |    written    |    token      |    scanned
 *          |               |               |
 *          outp            inp             p
 *
 *  *outp   first char not yet written to output file
 *  *inp    first char of current token
 *  *p      first char not yet scanned
 *
 * macro expansion: write from *outp to *inp (chars waiting to be written),
 * ignore from *inp to *p (chars of the macro call), place generated
 * characters in front of *p (in reverse order), update pointers,
 * resume scanning.
 *
 * symbol table pointers point to just beyond the end of macro definitions;
 * the first preceding character is the number of formal parameters.
 * the appearance of a formal in the body of a definition is marked by
 * 2 chars: the char WARN, and a char containing the parameter number.
 * the first char of a definition is preceded by a zero character.
 *
 * when macro expansion attempts to back up over the beginning of the
 * buffer, some characters preceding *pend are saved in a side buffer,
 * the address of the side buffer is put on 'instack', and the rest
 * of the main buffer is moved to the right.  the end of the saved buffer
 * is kept in 'endbuf' since there may be nulls in the saved buffer.
 *
 * similar action is taken when an 'include' statement is processed,
 * except that the main buffer must be completely emptied.  the array
 * element 'inctop[ifno]' records the last side buffer saved when
 * file 'ifno' was included.  these buffers remain dormant while
 * the file is being read, and are reactivated at end-of-file.
 *
 * instack[0 : mactop] holds the addresses of all pending side buffers.
 * instack[inctop[ifno]+1 : mactop-1] holds the addresses of the side
 * buffers which are "live"; the side buffers instack[0 : inctop[ifno]]
 * are dormant, waiting for end-of-file on the current file.
 *
 * space for side buffers is obtained from 'savch' and is never returned.
 * bufstack[0:fretop-1] holds addresses of side buffers which
 * are available for use.
 */

/* write part of buffer which lies between  outp  and  inp .
 * this should be a direct call to 'write', but the system slows to a crawl
 * if it has to do an unaligned copy.  thus we buffer.  this silly loop
 * is 15% of the total time, thus even the 'putc' macro is too slow.
 */
void
dump()
{
	register char *p1;
	register FILE *f;

	p1 = outp;
	if (p1 == inp || flslvl != 0)
		return;
	f = fout;
	while (p1 < inp)
		putc(*p1++, f);
	outp = p1;
}

/* dump buffer.  save chars from inp to p.  read into buffer at pbuf,
 * contiguous with p.  update pointers, return new p.
 */
char *
refill(p)
	register char *p;
{
	register char *np, *op;
	register int ninbuf;

	dump();
	np = pbuf - (p-inp);
	op = inp;
	if (bob (np + 1)) {
		pperror("token too long");
		np = pbeg;
		p = inp + BUFSIZ;
	}
	macdam += np-inp;
	outp = inp = np;
	while (op<p)
		*np++ = *op++;
	p = np;
	for (;;) {
		if (mactop > inctop[ifno]) {
			/* retrieve hunk of pushed-back macro text */
			op = instack[--mactop];
			np = pbuf;
			do {
				while ((*np++ = *op++));
			} while (op < endbuf[mactop]);
			pend = np-1;
			/* make buffer space avail for 'include' processing */
			if (fretop < MAXFRE)
				bufstack[fretop++] = instack[mactop];
			return p;
		}
		/* get more text from file(s) */
		maclvl = 0;
		ninbuf = read(fin, pbuf, BUFSIZ);
		if (ninbuf > 0) {
			pend = pbuf + ninbuf;
			*pend = '\0';
			return p;
		}
		/* end of #include file */
		if (ifno == 0) {
			/* end of input */
			if (plvl != 0) {
				int n = plvl, tlin = lineno[ifno];
				char *tfil = fnames[ifno];

				lineno[ifno] = maclin;
				fnames[ifno] = macfil;
				pperror("%s: unterminated macro call", macnam);
				lineno[ifno] = tlin;
				fnames[ifno] = tfil;
				np = p;
				/* shut off unterminated quoted string */
				*np++ = '\n';
				/* supply missing parens */
				while (--n >= 0)
					*np++ = ')';
				pend = np;
				*np = '\0';
				if (plvl < 0)
					plvl = 0;
				return p;
			}
			if (trulvl || flslvl)
				pperror("missing endif");
			inp = p;
			dump();
			exit(exfail);
		}
		close(fin);
		fin = fins[--ifno];
		dirs[0] = dirnams[ifno];
		sayline(BACK);
	}
}

#define BEG 0
#define LF 1

char *
cotoken(p)
	register char *p;
{
	register int c, i;
	char quoc;
	static int state = BEG;

	if (state != BEG)
		goto prevlf;
	for (;;) {
again:		while (! isspc(*p++));

		inp = p - 1;
		switch (*inp) {
		case 0:
			if (eob(--p)) {
				p = refill(p);
				goto again;
			}
			++p; /* ignore null byte */
			break;

		case '|': case '&':
			for (;;) {/* sloscan only */
				if (*p++ == *inp)
					break;
				if (! eob(--p))
					break;
				p = refill(p);
			}
			break;

		case '=': case '!':
			for (;;) {/* sloscan only */
				if (*p++ == '=')
					break;
				if (! eob(--p))
					break;
				p = refill(p);
			}
			break;

		case '<': case '>':
			for (;;) {/* sloscan only */
				if (*p++ == '=' || p[-2] == p[-1])
					break;
				if (! eob(--p))
					break;
				p = refill(p);
			}
			break;

		case '\\':
			for (;;) {
				if (*p++ == '\n') {
					++lineno[ifno];
					break;
				}
				if (! eob(--p)) {
					++p;
					break;
				}
				p = refill(p);
			}
			break;

		case '/':
			for (;;) {
				if (*p++ == '*') {/* comment */
					if (! passcom) {
						inp = p - 2;
						dump();
						++flslvl;
					}
					for (;;) {
						while (!iscom(*p++));
						if (p[-1] == '*') {
							for (;;) {
								if (*p++ == '/')
									goto endcom;
								if (! eob(--p))
									break;
								if (! passcom) {
									inp = p;
									p = refill(p);
								} else if ((p - inp) >= BUFSIZ) {
									/* split long comment */
									inp = p;
									/* last char written is '*' */
									p = refill(p);
									/* terminate first part */
									cputc('/', fout);
									/* and fake start of 2nd */
									outp = inp = p -= 3;
									*p++ = '/';
									*p++ = '*';
									*p++ = '*';
								} else
									p = refill(p);
							}
						} else if (p[-1] == '\n') {
							++lineno[ifno];
							if (! passcom)
								putc('\n', fout);
						} else if (eob(--p)) {
							if (! passcom) {
								inp = p;
								p = refill(p);
							} else if ((p - inp) >= BUFSIZ) {
								/* split long comment */
								inp = p;
								p = refill(p);
								cputc('*', fout);
								cputc('/', fout);
								outp = inp = p -= 2;
								*p++ = '/';
								*p++ = '*';
							} else
								p = refill(p);
						} else
							++p; /* ignore null byte */
					}
endcom:					if (! passcom) {
						outp = inp = p;
						--flslvl;
						goto again;
					}
					break;
				}
				if (! eob(--p))
					break;
				p = refill(p);
			}
			break;

		case '"': case '\'':
			quoc = p[-1];
			for (;;) {
				while (! isquo(*p++));
				if (p[-1] == quoc)
					break;
				if (p[-1] == '\n') {
					/* bare \n terminates quotation */
					--p;
					break;
				}
				if (p[-1] == '\\') {
					for (;;) {
						if (*p++ == '\n') {
							/* escaped \n ignored */
							++lineno[ifno];
							break;
						}
						if (! eob(--p)) {
							++p;
							break;
						}
						p = refill(p);
					}
				} else if (eob(--p))
					p = refill(p);
				else
					++p;	/* it was a different quote character */
			}
			break;

		case '\n':
			++lineno[ifno];
			if (isslo) {
				state = LF;
				return p;
			}
prevlf:
			state = BEG;
			for (;;) {
				if (*p++ == '#')
					return p;
				inp = --p;
				if (! eob(inp))
					goto again;
				p = refill(p);
			}
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (;;) {
				while (isnum(*p++));
				if (! eob(--p))
					break;
				p = refill(p);
			}
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z': case '_':
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
#define tmac1(c,bit) if (!xmac1(c,bit,&)) goto nomac
#define xmac1(c,bit,op) ((macbit+COFF)[c] op (bit))

			if (flslvl)
				goto nomac;
			for (;;) {
				c = p[-1];                          tmac1(c,b0);
				i = *p++; if (!isid(i)) goto endid; tmac1(i,b1);
				c = *p++; if (!isid(c)) goto endid; tmac1(c,b2);
				i = *p++; if (!isid(i)) goto endid; tmac1(i,b3);
				c = *p++; if (!isid(c)) goto endid; tmac1(c,b4);
				i = *p++; if (!isid(i)) goto endid; tmac1(i,b5);
				c = *p++; if (!isid(c)) goto endid; tmac1(c,b6);
				i = *p++; if (!isid(i)) goto endid; tmac1(i,b7);

				while (isid(*p++));
				if (eob(--p)) {
					refill(p);
					p = inp + 1;
					continue;
				}
				goto lokid;
endid:
				if (eob(--p)) {
					refill(p);
					p = inp + 1;
					continue;
				}
lokid:
				slookup(inp,p,0);
				if (newp) {
					p = newp;
					goto again;
				}
				break;
nomac:
				while (isid(*p++));
				if (eob(--p)) {
					p = refill(p);
					goto nomac;
				}
				break;
			}
			break;
		} /* end of switch */

		if (isslo)
			return p;
	} /* end of infinite loop */
}

/* get next non-blank token */
char *
skipbl(p)
	register char *p;
{
	do {
		outp = inp = p;
		p = cotoken(p);
	} while ((toktyp + COFF)[(int)*inp] == BLANK);
	return p;
}

/* take <= BUFSIZ chars from right end of buffer and put them on instack .
 * slide rest of buffer to the right, update pointers, return new p.
 */
char *
unfill(p)
	register char *p;
{
	register char *np, *op;
	register int d;

	if (mactop >= MAXFRE) {
		pperror("%s: too much pushback", macnam);
		/* begin flushing pushback */
		p = inp = pend;
		dump();
		while (mactop > inctop[ifno]) {
			p = refill(p);
			p = inp = pend;
			dump();
		}
	}
	if (fretop > 0)
		np = bufstack[--fretop];
	else {
		np = savch;
		savch += BUFSIZ;
		if (savch >= sbf + SBSIZE) {
			pperror("no space");
			exit(exfail);
		}
		*savch++ = '\0';
	}
	instack[mactop] = np;
	op = pend - BUFSIZ;
	if (op < p)
		op = p;
	for (;;) {
		while ((*np++ = *op++));
		if (eob(op))
			break;
	} /* out with old */
	endbuf[mactop++] = np;	/* mark end of saved text */
	np = pbuf + BUFSIZ;
	op = pend - BUFSIZ;
	pend = np;
	if (op < p)
		op = p;
	while (outp < op)
		*--np = *--op; /* slide over new */
	if (bob(np))
		pperror("token too long");
	d = np - outp;
	outp += d;
	inp += d;
	macdam += d;
	return p + d;
}

char *
doincl(p)
	register char *p;
{
	int filok, inctype;
	register char *cp;
	char **dirp, *nfil;
	char filname[BUFSIZ];

	p = skipbl(p);
	cp = filname;
	if (*inp++ == '<') {/* special <> syntax */
		inctype = 1;
		++flslvl;	/* prevent macro expansion */
		for (;;) {
			outp = inp = p;
			p = cotoken(p);
			if (*inp == '\n') {
				--p;
				*cp = '\0';
				break;
			}
			if (*inp == '>') {
				*cp = '\0';
				break;
			}
			while (inp < p)
				*cp++ = *inp++;
		}
		--flslvl;	/* reenable macro expansion */
	} else if (inp[-1] == '"') {/* regular "" syntax */
		inctype = 0;
		while (inp < p)
			*cp++ = *inp++;
		if (*--cp == '"')
			*cp = '\0';
	} else {
		pperror("bad include syntax", 0);
		inctype = 2;
	}
	/* flush current file to \n , then write \n */
	++flslvl;
	do {
		outp = inp = p;
		p = cotoken(p);
	} while (*inp != '\n');
	--flslvl;
	inp = p;
	dump();
	if (inctype == 2)
		return p;
	/* look for included file */
	if (ifno+1 >= MAXINC) {
		pperror("Unreasonable include nesting", 0);
		return p;
	}
	nfil = savch;
	if (nfil > sbf + SBSIZE - BUFSIZ) {
		pperror("no space");
		exit(exfail);
	}
	filok = 0;
	for (dirp = dirs + inctype; *dirp; ++dirp) {
		if (filname[0] == '/' || **dirp == '\0')
			strcpy(nfil, filname);
		else {
			strcpy(nfil, *dirp);
			strcat(nfil, "/");
			strcat(nfil, filname);
		}
		fins[ifno+1] = open(nfil, O_RDONLY);
		if (fins[ifno+1] > 0) {
			filok = 1;
			fin = fins[++ifno];
			break;
		}
	}
	if (filok == 0)
		pperror("Can't find include file %s", filname);
	else {
		lineno[ifno] = 1;
		fnames[ifno] = cp = nfil;
		while (*cp++);
		savch = cp;
		dirnams[ifno] = dirs[0] = trmdir(copy(nfil));
		sayline(START);
		/* save current contents of buffer */
		while (! eob(p))
			p = unfill(p);
		inctop[ifno] = mactop;
	}
	return p;
}

int
equfrm(a, p1, p2)
	register char *a, *p1, *p2;
{
	register char c;
	int flag;

	c = *p2; *p2 = '\0';
	flag = strcmp(a, p1);
	*p2 = c;
	return flag == SAME;
}

/* process '#define' */
char *
dodef(p)
	char *p;
{
	register char *pin, *psav, *cf;
	char **pf, **qf;
	int b, c, params;
	struct symtab *np;
	char *oldval, *oldsavch;
	char *formal[MAXFRM]; /* formal[n] is name of nth formal */
	char formtxt[BUFSIZ]; /* space for formal names */
	int spasscom = passcom;

	pf = 0;
	passcom = 0;		/* strip comments from defines to save space */

	if (savch > sbf + SBSIZE - BUFSIZ) {
		pperror("too much defining");
		return p;
	}
	oldsavch = savch; /* to reclaim space if redefinition */
	++flslvl; /* prevent macro expansion during 'define' */
	p = skipbl(p);
	pin = inp;
	if ((toktyp + COFF)[(int)*pin] != IDENT) {
		ppwarn("illegal macro name");
		while (*inp != '\n')
			p = skipbl(p);
		return p;
	}
	np = slookup(pin, p, 1);
	oldval = np->value;
	if (oldval)
		savch = oldsavch;	/* was previously defined */
	b = 1;
	cf = pin;
	while (cf < p) {/* update macbit */
		c = *cf++;
		xmac1(c, b, |=);
		b = (b + b) & 0xFF;
	}
	params = 0;
	outp = inp = p;
	p = cotoken(p);
	pin = inp;
	if (*pin == '(') {/* with parameters; identify the formals */
		cf = formtxt;
		pf = formal;
		for (;;) {
			p = skipbl(p);
			pin = inp;
			if (*pin == '\n') {
				--lineno[ifno];
				--p;
				pperror("%s: missing )", np->name);
				break;
			}
			if (*pin == ')')
				break;
			if (*pin == ',')
				continue;
			if ((toktyp + COFF)[(int)*pin] != IDENT) {
				c = *p;
				*p = '\0';
				pperror("bad formal: %s", pin);
				*p = c;
			} else if (pf >= &formal[MAXFRM]) {
				c = *p;
				*p = '\0';
				pperror("too many formals: %s", pin);
				*p = c;
			} else {
				*pf++ = cf;
				while (pin < p)
					*cf++ = *pin++;
				*cf++ = '\0';
				++params;
			}
		}
		if (params == 0)
			--params; /* #define foo() ... */
	} else if (*pin == '\n') {
		--lineno[ifno];
		--p;
	}
	/* remember beginning of macro body, so that we can
	 * warn if a redefinition is different from old value.
	 */
	oldsavch = psav = savch;
	for (;;) {/* accumulate definition until linefeed */
		outp = inp = p;
		p = cotoken(p);
		pin = inp;
		if (*pin == '\\' && pin[1] == '\n') {
			/* ignore escaped lf */
			putc('\n', fout);
			continue;
		}
		if (*pin == '\n')
			break;
		if (params) {/* mark the appearance of formals in the definiton */
			if ((toktyp + COFF)[(int)*pin] == IDENT) {
				for (qf = pf; --qf >= formal; ) {
					if (equfrm(*qf, pin, p)) {
						*psav++ = qf - formal + 1;
						*psav++ = WARN;
						pin = p;
						break;
					}
				}
			} else if (*pin == '"' || *pin == '\'') {
				/* inside quotation marks, too */
				char quoc = *pin;

				*psav++ = *pin++;
				while (pin < p && *pin != quoc) {
					while (pin < p && !isid(*pin))
						*psav++ = *pin++;
					cf = pin;
					while (cf < p && isid(*cf))
						++cf;
					for (qf = pf; --qf >= formal; ) {
						if (equfrm(*qf, pin, cf)) {
							*psav++ = qf-formal+1;
							*psav++ = WARN;
							pin = cf;
							break;
						}
					}
					while (pin < cf)
						*psav++ = *pin++;
				}
			}
		}
		while (pin < p)
			*psav++ = *pin++;
	}
	*psav++ = params;
	*psav++ = '\0';
	cf = oldval;
	if (cf != NULL) {/* redefinition */
		--cf;	/* skip no. of params, which may be zero */
		while (*--cf);	/* go back to the beginning */
		if (0 != strcmp(++cf, oldsavch)) {
			/* redefinition different from old */
			--lineno[ifno];
			ppwarn("%s redefined", np->name);
			++lineno[ifno];
			np->value = psav - 1;
		} else
			psav = oldsavch; /* identical redef.; reclaim space */
	} else
		np->value = psav - 1;
	--flslvl;
	inp = pin;
	savch = psav;
	passcom = spasscom;
	return p;
}

char *
savestring(start, finish)
	register char *start, *finish;
{
	char *retbuf;
	register char *cp;

	retbuf = (char *) calloc(finish - start + 1, sizeof (char));
	cp = retbuf;
	while (start < finish)
		*cp++ = *start++;
	*cp = 0;
	return retbuf;
}

#define fasscan() ptrtab = fastab + COFF
#define sloscan() ptrtab = slotab + COFF

/* find and handle preprocessor control lines */
char *
control(p)
	register char *p;
{
	register struct symtab *np;

	for (;;) {
		fasscan();
		p = cotoken(p);
		if (*inp == '\n')
			++inp;
		dump();
		sloscan();
		p = skipbl(p);
		*--inp = SALT;
		outp = inp;
		++flslvl;
		np = slookup(inp, p, 0);
		--flslvl;
		if (np == defloc) {/* define */
			if (flslvl == 0) {
				p = dodef(p);
				continue;
			}
		} else if (np == incloc) {/* include */
			if (flslvl == 0) {
				p = doincl(p);
				continue;
			}
		} else if (np == ifnloc) {/* ifndef */
			++flslvl;
			p = skipbl(p);
			np = slookup(inp, p, 0);
			--flslvl;
			if (flslvl == 0 && np->value == 0)
				++trulvl;
			else
				++flslvl;
		} else if (np == ifdloc) {/* ifdef */
			++flslvl;
			p = skipbl(p);
			np = slookup(inp, p, 0);
			--flslvl;
			if (flslvl == 0 && np->value != 0)
				++trulvl;
			else
				++flslvl;
		} else if (np == eifloc) {/* endif */
			if (flslvl) {
				if (--flslvl==0)
					sayline(CONT);
			} else if (trulvl)
				--trulvl;
			else
				pperror("If-less endif", 0);
		} else if (np == elsloc) {/* else */
			if (flslvl) {
				if (--flslvl != 0)
					++flslvl;
				else {
					++trulvl;
					sayline(CONT);
				}
			}
			else if (trulvl) {
				++flslvl;
				--trulvl;
			} else
				pperror("If-less else", 0);
		} else if (np == udfloc) {/* undefine */
			if (flslvl == 0) {
				++flslvl;
				p = skipbl(p);
				slookup(inp, p, DROP);
				--flslvl;
			}
		} else if (np == ifloc) {/* if */
			newp = p;
			if (flslvl == 0 && yyparse())
				++trulvl;
			else
				++flslvl;
			p = newp;
		} else if (np == lneloc) {/* line */
			if (flslvl == 0 && pflag == 0) {
				char *cp, *cp2;

				outp = inp = p;
				*--outp='#';
				while (*inp != '\n')
					p = cotoken(p);
				cp = outp + 1;
				while (isspace(*cp) && cp < inp)
					cp++;
				while (isdigit(*cp) && cp < inp)
					cp++;
				while (*cp != '"' && cp < inp)
					cp++;
				if (cp < inp) {
					cp++;
					cp2 = cp;
					while (*cp2 != '"' && cp2 < inp)
						cp2++;
					fnames[ifno] = savestring(cp, cp2);
				}
				continue;
			}
		} else if (*++inp == '\n')
			outp = inp;	/* allows blank line after # */
		else
			pperror("undefined control", 0);
		/* flush to lf */
		++flslvl;
		while (*inp != '\n') {
			outp = inp = p;
			p = cotoken(p);
		}
		--flslvl;
	}
}

struct symtab *
stsym(s)
	register char *s;
{
	char buf[BUFSIZ];
	register char *p;

	/* make definition look exactly like end of #define line */
	/* copy to avoid running off end of world when param list is at end */
	p = buf;
	while ((*p++ = *s++));
	p = buf;
	while (isid(*p++)); /* skip first identifier */
	if (*--p == '=') {
		*p++ = ' ';
		while (*p++);
	} else {
		s = " 1";
		while ((*p++ = *s++));
	}
	pend = p;
	*--p='\n';
	sloscan();
	dodef(buf);
	return lastsym;
}

/* kluge */
struct symtab *
ppsym(s)
	char *s;
{
	register struct symtab *sp;

	cinit = SALT;
	*savch++ = SALT;
	sp = stsym(s);
	--sp->name;
	cinit = 0;
	return sp;
}

struct symtab *
lookup(namep, enterf)
	char *namep;
{
	register char *np, *snp;
	register int c, i;
	int around;
	register struct symtab *sp;

	/* namep had better not be too long (currently, <=NCPS chars) */
	np = namep;
	around = 0;
	i = cinit;
	while ((c = *np++))
		i += i + c;
	c = i;	/* c=i for register usage on pdp11 */
	c %= symsiz;
	if (c < 0)
		c += symsiz;
	sp = &stab[c];
	while ((snp = sp->name)) {
		np = namep;
		while (*snp++ == *np) {
			if (*np++ == '\0') {
				if (enterf == DROP) {
					sp->name[0] = DROP;
					sp->value = 0;
				}
				lastsym = sp;
				return lastsym;
			}
		}
		if (--sp < &stab[0]) {
			if (around) {
				pperror("too many defines", 0);
				exit(exfail);
			} else {
				++around;
				sp = &stab[symsiz-1];
			}
		}
	}
	if (enterf == 1)
		sp->name = namep;
	lastsym = sp;
	return lastsym;
}

struct symtab *
slookup(p1, p2, enterf)
	register char *p1,*p2;
	int enterf;
{
	register char *p3;
	char c2,c3;
	struct symtab *np;

	c2 = *p2;
	*p2 = '\0';	/* mark end of token */
	if ((p2 - p1) > NCPS)
		p3 = p1 + NCPS;
	else
		p3 = p2;
	c3 = *p3;
	*p3 = '\0';	/* truncate to NCPS chars or less */
	if (enterf == 1)
		p1 = copy(p1);
	np = lookup(p1, enterf);
	*p3 = c3;
	*p2 = c2;
	if (np->value != 0 && flslvl == 0)
		newp = subst(p2, np);
	else
		newp = 0;
	return np;
}

char *
subst(p, sp)
	register char *p;
	struct symtab *sp;
{
	static char match[] = "%s: argument mismatch";
	register char *ca, *vp;
	int params;
	char *actual[MAXFRM]; /* actual[n] is text of nth actual   */
	char actused[MAXFRM]; /* for newline processing in actuals */
	char acttxt[BUFSIZ];  /* space for actuals */
	int nlines = 0;

	vp = sp->value;
	if (vp == 0)
		return p;
	if ((p - macforw) <= macdam) {
		if (++maclvl > symsiz && ! rflag) {
			pperror("%s: macro recursion", sp->name);
			return p;
		}
	} else
		maclvl = 0;	/* level decreased */
	macforw = p;
	macdam = 0;		/* new target for decrease in level */
	macnam = sp->name;
	dump();
	if (sp == ulnloc) {
		vp = acttxt;
		*vp++ = '\0';
		sprintf(vp, "%d", lineno[ifno]);
		while (*vp++);
	} else if (sp == uflloc) {
		vp = acttxt;
		*vp++ = '\0';
		sprintf(vp, "\"%s\"", fnames[ifno]);
		while (*vp++);
	}
	params = *--vp & 0xFF;
	if (params != 0) {	/* definition calls for params */
		register char **pa;

		ca = acttxt;
		pa = actual;
		if (params == 0xFF)
			params = 1;	/* #define foo() ... */
		sloscan();
		++flslvl;	/* no expansion during search for actuals */
		plvl = -1;
		do {
			p = skipbl(p);
		} while (*inp == '\n');	/* skip \n too */

		if (*inp == '(') {
			maclin = lineno[ifno];
			macfil = fnames[ifno];
			for (plvl = 1; plvl != 0; ) {
				*ca++ = '\0';
				for (;;) {
					outp = inp = p;
					p = cotoken(p);
					if (*inp == '(')
						++plvl;
					if (*inp == ')' && --plvl == 0) {
						--params;
						break;
					}
					if (plvl == 1 && *inp == ',') {
						--params;
						break;
					}
					while (inp < p)
						*ca++= *inp++;
					if (ca > &acttxt[BUFSIZ])
						pperror("%s: actuals too long", sp->name);
				}
				if (pa >= &actual[MAXFRM])
					ppwarn(match, sp->name);
				else {
					actused[pa-actual] = 0;
					*pa++ = ca;
				}
			}
			nlines = lineno[ifno] - maclin;
			lineno[ifno] = maclin; /* don't count newlines here */
		}
		if (params != 0)
			ppwarn(match, sp->name);
		while (--params >= 0)
			*pa++ = "" + 1;	/* null string for missing actuals */
		--flslvl;
		fasscan();
	}
	for (;;) {/* push definition onto front of input stack */
		while (! iswarn(*--vp)) {
			if (bob(p)) {
				outp = inp = p;
				p = unfill(p);
			}
			*--p = *vp;
		}
		if (*vp == warnc) {/* insert actual param */
			ca = actual[*--vp-1];
			while (*--ca) {
				if (bob(p)) {
					outp = inp = p;
					p = unfill(p);
				}
				/* Actuals with newlines confuse line numbering */
				if (*ca == '\n' && actused[*vp-1])
					if (*(ca-1) == '\\')
						ca--;
					else
						*--p = ' ';
				else {
					*--p = *ca;
					if (*ca == '\n')
						nlines--;
				}
			}
			actused[*vp-1] = 1;
		} else {
			if (nlines > 0 )
				while (nlines-- > 0)
					*--p = '\n';
			break;
		}
	}
	outp = inp = p;
	return p;
}

char *
trmdir(s)
	register char *s;
{
	register char *p = s;

	while (*p++);
	--p;
	while (p>s && *--p != '/');
	if (p == s)
		*p++ = '.';
	*p = '\0';
	return s;
}

STATIC char *
copy(s)
	register char *s;
{
	register char *old;

	old = savch;
	while ((*savch++ = *s++));
	return old;
}

char *
strdex(s, c)
	char *s, c;
{
	while (*s)
		if (*s++ == c)
			return --s;
	return 0;
}

int
yywrap()
{
	return 1;
}

int
main(argc, argv)
	char **argv;
{
	register int i, c;
	register char *p;
	char *tf, **cp2, obuf[BUFSIZ];

	p = "_$ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	i = 0;
	while ((c = *p++)) {
		(fastab + COFF)[c] |= IB | NB | SB;
		(toktyp + COFF)[c] = IDENT;
	}
	p = "0123456789.";
	while ((c = *p++)) {
		(fastab + COFF)[c] |= NB | SB;
		(toktyp + COFF)[c] = NUMBR;
	}
	p = "\n\"'/\\";
	while ((c = *p++))
		(fastab + COFF)[c] |= SB;
	p = "\n\"'\\";
	while ((c = *p++))
		(fastab + COFF)[c] |= QB;
	p = "*\n";
	while ((c = *p++))
		(fastab + COFF)[c] |= CB;

	(fastab + COFF)[(int)warnc] |= WB;
	(fastab + COFF)['\0'] |= CB | QB | SB | WB;
	for (i = ALFSIZ; --i >= 0; )
		slotab[i] = fastab[i] | SB;

	/* note no \n;	\v not legal for vertical tab? */
	p = " \t\013\f\r";
	while ((c = *p++))
		(toktyp + COFF)[c] = BLANK;

	ifno = 0;
	fnames[0] = "";
	dirnams[0] = dirs[0] = ".";
	fout = stdout;
	for (i=1; i<argc; i++) {
		switch (argv[i][0]) {
		case '-':
			switch (argv[i][1]) {
			case 'M':
				mflag++;
			case 'P':
				pflag++;
			case 'E':
			case '\0':
				continue;

			case 'R':
				++rflag;
				continue;

			case 'C':
				passcom++;
				continue;

			case 'D':
				if (predef > prespc + NPREDEF) {
					pperror("too many -D options, ignoring %s", argv[i]);
					continue;
				}
				/* ignore plain "-D" (no argument) */
				if (argv[i][2])
					*predef++ = &argv[i][2];
				continue;

			case 'U':
				if (prund > punspc + NPREDEF) {
					pperror("too many -U options, ignoring %s", argv[i]);
					continue;
				}
				*prund++ = &argv[i][2];
				continue;

			case 'I':
				if (nd > 8)
					pperror("excessive -I file (%s) ignored", argv[i]);
				else
					dirs[nd++] = &argv[i][2];
				continue;

			case 'o':
				fclose(fout);
				if (argv[i][2]) {
					p = &argv[i][2];
				} else {
					p = argv[++i];
				}
				fout = fopen(p, "w");
				if (! fout) {
					pperror("Can't create %s", p);
					exit(8);
				}
				continue;

			default:
				pperror("unknown flag %s", argv[i]);
				continue;
			}
		default:
			if (fin != STDIN) {
				pperror("extraneous name %s", argv[i]);
				continue;
			}
			fin = open(argv[i], O_RDONLY);
			if (fin < 0) {
				pperror("No source file %s", argv[i]);
				exit(8);
			}
			fnames[ifno] = copy(argv[i]);
			infile = copy(argv[i]);
			dirs[0] = dirnams[ifno] = trmdir(argv[i]);
		}
	}
	if (mflag) {
		if (! infile) {
			fprintf(stderr,
				"no input file specified with -M flag\n");
			exit(8);
		}
		tf = (char*) rindex(infile, '.');
		if (! tf) {
			fprintf(stderr, "missing component name on %s\n",
				infile);
			exit(8);
		}
		tf[1] = 'o';
		tf = (char*) rindex(infile, '/');
		if (tf)
			infile = tf + 1;
		mout = fout;
		setbuf(mout, 0);
		fout = fopen("/dev/null", "w");
		if (! fout) {
			pperror("Can't open /dev/null");
			exit(8);
		}
	}
	setbuf(fout, obuf);
	fins[ifno] = fin;
	exfail = 0;

	/* after user -I files here are the standard include libraries */
	dirs[nd++] = "/usr/include";
	dirs[nd++] = 0;
	defloc = ppsym("define");
	udfloc = ppsym("undef");
	incloc = ppsym("include");
	elsloc = ppsym("else");
	eifloc = ppsym("endif");
	ifdloc = ppsym("ifdef");
	ifnloc = ppsym("ifndef");
	ifloc = ppsym("if");
	lneloc = ppsym("line");
	for (i = sizeof(macbit) / sizeof(macbit[0]); --i >= 0; )
		macbit[i] =0;
	ulnloc = stsym ("__LINE__");
	uflloc = stsym ("__FILE__");

	tf = fnames[ifno];
	fnames[ifno] = "command line";
	lineno[ifno] = 1;

	cp2 = prespc;
	while (cp2 < predef)
		stsym(*cp2++);
	cp2 = punspc;
	while (cp2 < prund) {
		p = strdex(*cp2, '=');
		if (p)
			*p++ = '\0';
		lookup(*cp2++, DROP);
	}
	fnames[ifno] = tf;
	pbeg = buffer + NCPS;
	pbuf = pbeg + BUFSIZ;
	pend = pbuf + BUFSIZ;

	trulvl = 0;
	flslvl = 0;
	lineno[0] = 1;
	sayline(START);
	outp = inp = pend;
	control(pend);
	return exfail;
}

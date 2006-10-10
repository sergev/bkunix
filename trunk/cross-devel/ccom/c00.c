/* C compiler
 *
 * Called from cc:
 *   c0 source temp1 temp2 [ profileflag ]
 * temp1 gets most of the intermediate code;
 * strings are put on temp2, which c1 reads after temp1.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <string.h>
#include "c0.h"

int	isn	= 1;
int	peeksym	= -1;
int	line	= 1;
struct	tnode	funcblk = { NAME };

struct kwtab {
	char	*kwname;
	int	kwval;
} kwtab[] = {
	{ "int",	INT },
	{ "char",	CHAR },
	{ "float",	FLOAT },
	{ "double",	DOUBLE },
	{ "struct",	STRUCT },
	{ "long",	LONG },
	{ "unsigned",	UNSIGN },
	{ "union",	UNION },
	{ "short",	INT },
	{ "void",	VOID },
	{ "auto",	AUTO },
	{ "extern",	EXTERN },
	{ "static",	STATIC },
	{ "register",	REG },
	{ "goto",	GOTO },
	{ "return",	RETURN },
	{ "if",		IF },
	{ "while",	WHILE },
	{ "else",	ELSE },
	{ "switch",	SWITCH },
	{ "case",	CASE },
	{ "break",	BREAK },
	{ "continue",	CONTIN },
	{ "do",		DO },
	{ "default",	DEFAULT },
	{ "for",	FOR },
	{ "sizeof",	SIZEOF },
	{ "typedef",	TYPEDEF },
	{ "enum",	ENUM },
	{ "asm",	ASM },
	{ 0,		0 },
};

union	tree *cmst[CMSIZ];
union	tree **cp = cmst;
int	Wflag;			/* print warning messages */

int
main(argc, argv)
int	argc;
char	*argv[];
{
	register unsigned i;
	register struct kwtab *ip;
	char	buf1[BUFSIZ],
		buf2[BUFSIZ];

	if (argc>1 && strcmp(argv[1], "-u")==0) {
		argc--;
		argv++;
		unscflg++;
	}
	if(argc<4) {
		error("Arg count");
		exit(1);
	}
	if (freopen(argv[1], "r", stdin)==NULL) {
		error("Can't find %s", argv[1]);
		exit(1);
	}
	setbuf(stdin,buf1);	/* stdio sbrk problems */
	if (freopen(argv[2], "w", stdout)==NULL || (sbufp=fopen(argv[3],"w"))==NULL) {
		error("Can't create temp");
		exit(1);
	}
	setbuf(stdout,buf2);	/* stdio sbrk problems */
	setbuf(sbufp, sbuf);
	/*
	 * Working without overlays, frame does not have overlay number
	 */
	STAUTO = -6;
	while (argc>4) {
		switch (argv[4][1]) {
		case 'P':
			proflg++;
			break;
		case 'V':		/* overlays */
			/* allow an extra word on the stack for
			 * each stack from to store the overlay number. */
			STAUTO -= 2;
			break;
		case 'w':
		case 'W':		/* don't print warning messages */
			Wflag++;
			break;
		}
		argc--; argv++;
	}
	/*
	 * The hash table locations of the keywords
	 * are marked; if an identifier hashes to one of
	 * these locations, it is looked up in in the keyword
	 * table first.
	 */
	for (ip=kwtab; ip->kwname; ip++) {
		i = hash(ip->kwname);
		kwhash[i/LNBPW] |= 1 << (i%LNBPW);
	}
	coremax = locbase = sbrk(0);
	while(!eof)
		extdef();
	outcode("B", EOFC);
	strflg++;
	outcode("B", EOFC);
	blkend();
	exit(nerror!=0);
}

/*
 * Look up the identifier in symbuf in the symbol table.
 * If it hashes to the same spot as a keyword, try the keyword table
 * first.
 * Return is a ptr to the symbol table entry.
 */
int
lookup()
{
	unsigned ihash;
	register struct nmlist *rp;

	ihash = hash(symbuf);
	if (kwhash[ihash/LNBPW] & (1 << (ihash%LNBPW)))
		if (findkw())
			return(KEYW);
	rp = hshtab[ihash];
	while (rp) {
		if (strcmp(symbuf, rp->name) != 0)
			goto no;
		if (mossym != (rp->hflag&FKIND))
			goto no;
		csym = rp;
		return(NAME);
	no:
		rp = rp->nextnm;
	}
	rp = (struct nmlist *)Dblock(sizeof(struct nmlist));
	rp->nextnm = hshtab[ihash];
	hshtab[ihash] = rp;
	rp->hclass = 0;
	rp->htype = 0;
	rp->hoffset = 0;
	rp->hsubsp = NULL;
	rp->hstrp = NULL;
	rp->sparent = NULL;
	rp->hblklev = blklev;
	rp->hflag = mossym;
	rp->name = Dblock((strlen(symbuf) + 1 + LNCPW - 1) & ~(LNCPW - 1));
	strcpy(rp->name, symbuf);
	csym = rp;
	return(NAME);
}

/*
 * Search the keyword table.
 */
int
findkw()
{
	register struct kwtab *kp;

	for (kp=kwtab; kp->kwname; kp++) {
		if (strcmp(symbuf, kp->kwname) == 0) {
			cval = kp->kwval;
			return(1);
		}
	}
	return(0);
}


/*
 * Return the next symbol from the input.
 * peeksym is a pushed-back symbol, peekc is a pushed-back
 * character (after peeksym).
 * mosflg means that the next symbol, if an identifier,
 * is a member of structure or a structure tag or an enum tag
 */
int
symbol()
{
	register int c;
	register char *sp;
	register int tline;

	if (peeksym>=0) {
		c = peeksym;
		peeksym = -1;
		if (c==NAME)
			mosflg = 0;
		return(c);
	}
	if (peekc) {
		c = peekc;
		peekc = 0;
	} else
		if (eof)
			return(EOFC);
		else
			c = getchar();
loop:
	if (c==EOF) {
		eof++;
		return(EOFC);
	}
	switch(ctab[c]) {

	case SHARP:
		if ((c=symbol())!=CON) {
			error("Illegal #");
			return(c);
		}
		tline = cval;
		while (ctab[peekc]==SPACE)
			peekc = getchar();
		if (peekc=='"') {
			sp = filename;
			while ((c = mapch('"')) >= 0)
				*sp++ = c;
			*sp++ = 0;
			peekc = getchar();
		}
		/* Skip cpp flags */
		while (ctab[peekc]==SPACE || ctab[peekc]==DIGIT)
			peekc = getchar();
		if (peekc != '\n') {
			error("Illegal #");
			while (getchar()!='\n' && eof==0)
				;
		}
		peekc = 0;
		line = tline;
		return(symbol());

	case NEWLN:
		line++;

	case SPACE:
		c = getchar();
		goto loop;

	case PLUS:
		return(subseq(c,PLUS,INCBEF));

	case MINUS:
		if (subseq(c, 0, 1))
			return(DECBEF);
		return(subseq('>', MINUS, ARROW));

	case ASSIGN:
		return(subseq(c, ASSIGN, EQUAL));

	case LESS:
		if (subseq(c,0,1))
			return(LSHIFT);
		return(subseq('=',LESS,LESSEQ));

	case GREAT:
		if (subseq(c,0,1))
			return(RSHIFT);
		return(subseq('=',GREAT,GREATEQ));

	case EXCLA:
		return(subseq('=',EXCLA,NEQUAL));

	case BSLASH:
		if (subseq('/', 0, 1))
			return(MAX);
		goto unkn;

	case DIVIDE:
		if (subseq('\\', 0, 1))
			return(MIN);
		if (subseq('*',1,0))
			return(DIVIDE);
		while ((c = spnextchar()) != EOFC) {
			peekc = 0;
			if (c=='*') {
				if (spnextchar() == '/') {
					peekc = 0;
					c = getchar();
					goto loop;
				}
			}
		}
		eof++;
		error("Nonterminated comment");
		return(0);

	case PERIOD:
	case DIGIT:
		peekc = c;
		return(getnum());

	case DQUOTE:
		cval = isn++;
		return(STRING);

	case SQUOTE:
		return(getcc());

	case LETTER:
		sp = symbuf;
		while (ctab[c]==LETTER || ctab[c]==DIGIT) {
			if (sp < symbuf + MAXCPS)
				*sp++ = c;
			c = getchar();
		}
		*sp++ = '\0';
		mossym = mosflg;
		mosflg = 0;
		peekc = c;
		if ((c=lookup())==KEYW && cval==SIZEOF)
			c = SIZEOF;
		return(c);

	case AND:
		return(subseq('&', AND, LOGAND));

	case OR:
		return(subseq('|', OR, LOGOR));

	case UNKN:
	unkn:
		error("Unknown character");
		c = getchar();
		goto loop;

	}
	return(ctab[c]);
}

/*
 * Read a number.  Return kind.
 */
int
getnum()
{
	register char *np;
	register int c, base;
	int expseen, sym, ndigit;
	char *nsyn;
	int maxdigit;

	nsyn = "Number syntax";
	lcval = 0;
	base = 10;
	maxdigit = 0;
	np = numbuf;
	ndigit = 0;
	sym = CON;
	expseen = 0;
	if ((c=spnextchar()) == '0')
		base = 8;
	for (;; c = getchar()) {
		*np++ = c;
		if (ctab[c]==DIGIT ||
		    ((base==16) && (('a'<=c&&c<='f') || ('A'<=c&&c<='F')))) {
			if (base==8)
				lcval <<= 3;
			else if (base==10)
				lcval = ((lcval<<2) + lcval)<<1;
			else
				lcval <<= 4;
			if (ctab[c]==DIGIT)
				c -= '0';
			else if (c>='a')
				c -= 'a'-10;
			else
				c -= 'A'-10;
			lcval += c;
			ndigit++;
			if (c>maxdigit)
				maxdigit = c;
			continue;
		}
		if (c=='.') {
			if (base==16 || sym==FCON)
				error(nsyn);
			sym = FCON;
			base = 10;
			continue;
		}
		if (ndigit==0) {
			sym = DOT;
			break;
		}
		if ((c=='e'||c=='E') && expseen==0) {
			expseen++;
			sym = FCON;
			if (base==16 || maxdigit>=10)
				error(nsyn);
			base = 10;
			*np++ = c = getchar();
			if (c!='+' && c!='-' && ctab[c]!=DIGIT)
				break;
		} else if (c=='x' || c=='X') {
			if (base!=8 || lcval!=0 || sym!=CON)
				error(nsyn);
			base = 16;
		} else if ((c=='l' || c=='L') && sym==CON) {
			c = getchar();
			sym = LCON;
			break;
		} else
			break;
	}
	peekc = c;
	if (maxdigit >= base)
		error(nsyn);
	if (sym==FCON) {
		np[-1] = 0;
		cval = np-numbuf;
		return(FCON);
	}
	if (sym==CON && (lcval<0 || (lcval>MAXINT&&base==10) || (lcval>>1)>MAXINT)) {
		sym = LCON;
	}
	cval = lcval;
	return(sym);
}

/*
 * If the next input character is c, return b and advance.
 * Otherwise push back the character and return a.
 */
int
subseq(c,a,b)
{
	if (spnextchar() != c)
		return(a);
	peekc = 0;
	return(b);
}

/*
 * Write out a string, either in-line
 * or in the string temp file labelled by
 * lab.
 */
void
putstr(lab, max)
register int max;
{
	register int c;

	nchstr = 0;
	if (lab) {
		strflg++;
		outcode("BNB", LABEL, lab, BDATA);
		max = 10000;
	} else
		outcode("B", BDATA);
	while ((c = mapch('"')) >= 0) {
		if (nchstr < max) {
			nchstr++;
			if (nchstr%15 == 0)
				outcode("0B", BDATA);
			outcode("1N", c & 0377);
		}
	}
	if (nchstr < max) {
		nchstr++;
		outcode("10");
	}
	outcode("0");
	strflg = 0;
}

void
cntstr()
{
	register int c;

	nchstr = 1;
	while ((c = mapch('"')) >= 0) {
		nchstr++;
	}
}

/*
 * read a single-quoted character constant.
 * The routine is sensitive to the layout of
 * characters in a word.
 */
int
getcc()
{
	register int c, cc;
	register char *ccp;
	char realc;

	cval = 0;
	ccp = (char *)&cval;
	cc = 0;
	while((c=mapch('\'')) >= 0)
		if(cc++ < LNCPW)
			*ccp++ = c;
	if (cc>LNCPW)
		error("Long character constant");
	if (cc==1) {
		realc = cval;
		cval = realc;
	}
	return(CON);
}

/*
 * Read a character in a string or character constant,
 * detecting the end of the string.
 * It implements the escape sequences.
 */
int
mapch(ac)
{
	register int a, c, n;
	static int mpeek;

	c = ac;
	if ((a = mpeek))
		mpeek = 0;
	else
		a = getchar();
loop:
	if (a==c)
		return(-1);
	switch(a) {

	case '\n':
	case '\0':
		error("Nonterminated string");
		peekc = a;
		return(-1);

	case '\\':
		switch (a=getchar()) {

		case 't':
			return('\t');

		case 'n':
			return('\n');

		case 'b':
			return('\b');

		case 'f':
			return('\014');

		case 'v':
			return('\013');

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			n = 0;
			c = 0;
			while (++c<=3 && '0'<=a && a<='7') {
				n <<= 3;
				n += a-'0';
				a = getchar();
			}
			mpeek = a;
			return(n);

		case 'r':
			return('\r');

		case '\n':
			line++;
			a = getchar();
			goto loop;
		}
	}
	return(a);
}

/*
 * Read an expression and return a pointer to its tree.
 * It's the classical bottom-up, priority-driven scheme.
 * The initflg prevents the parse from going past
 * "," or ":" because those delimiters are special
 * in initializer (and some other) expressions.
 */
union tree *
tree(eflag)
{
	int *op, opst[SSIZE], *pp, prst[SSIZE];
	register int andflg, o;
	register struct nmlist *cs;
	int p, ps, os, xo = 0, *xop;
	char *svtree;
	static struct cnode garbage = { CON, INT, (int *)NULL, (union str *)NULL, 0 };

	svtree = starttree();
	op = opst;
	pp = prst;
	*op = SEOF;
	*pp = 06;
	andflg = 0;

advanc:
	switch (o=symbol()) {

	case NAME:
		cs = csym;
		if (cs->hclass==TYPEDEF)
			goto atype;
		if (cs->hclass==ENUMCON) {
			*cp++ = cblock(cs->hoffset);
			goto tand;
		}
		if (cs->hclass==0 && cs->htype==0) {
			if(nextchar()=='(') {
				/* set function */
				cs->hclass = EXTERN;
				cs->htype = FUNC;
			} else {
				cs->hclass = STATIC;
				error("%s undefined; func. %s", cs->name,
					funcsym ? funcsym->name : "(none)");
			}
		}
		*cp++ = nblock(cs);
		goto tand;

	case FCON:
		*cp++ = fblock(DOUBLE, copnum(cval));
		goto tand;

	case LCON:
		*cp = (union tree *)Tblock(sizeof(struct lnode));
		(*cp)->l.op = LCON;
		(*cp)->l.type = LONG;
		(*cp)->l.lvalue = lcval;
		cp++;
		goto tand;

	case CON:
		*cp++ = cblock(cval);
		goto tand;

	/* fake a static char array */
	case STRING:
/*
 * This hack is to compensate for a bit of simplemindedness I'm not sure how
 * else to fix.
 *
 *	i = sizeof ("foobar");
 *
 * or
 *	i = sizeof "foobar";
 *
 * would generate ".byte 'f,'o','o,'b,'a,'r,0" into the data segment!
 *
 * What I did here was to scan to "operator" stack looking for left parens
 * "(" preceeded by a "sizeof".  If both are seen and in that order or only
 * a SIZEOF is sedn then the string is inside a 'sizeof' and should not
 * generate any data to the object file.
*/
		xop = op;
		while	(xop > opst)
			{
			xo = *xop--;
			if	(xo != LPARN)
				break;
			}
		if	(xo == SIZEOF)
			cntstr();
		else
			putstr(cval, 0);
		cs = (struct nmlist *)Tblock(sizeof(struct nmlist));
		cs->hclass = STATIC;
		cs->hoffset = cval;
		*cp++ = block(NAME, unscflg? ARRAY+UNCHAR:ARRAY+CHAR, &nchstr,
		  (union str *)NULL, (union tree *)cs, TNULL);

	tand:
		if(cp>=cmst+CMSIZ) {
			error("Expression overflow");
			exit(1);
		}
		if (andflg)
			goto syntax;
		andflg = 1;
		goto advanc;

	case KEYW:
	atype:
		if (*op != LPARN || andflg)
			goto syntax;
		peeksym = o;
		*cp++ = xprtype();
		if ((o=symbol()) != RPARN)
			goto syntax;
		o = CAST;
		--op;
		--pp;
		if (*op == SIZEOF) {
			andflg = 1;
			*pp = 100;
			goto advanc;
		}
		goto oponst;

	case INCBEF:
	case DECBEF:
		if (andflg)
			o += 2;
		goto oponst;

	case COMPL:
	case EXCLA:
	case SIZEOF:
		if (andflg)
			goto syntax;
		goto oponst;

	case MINUS:
		if (!andflg)
			o = NEG;
		andflg = 0;
		goto oponst;

	case AND:
	case TIMES:
		if (andflg)
			andflg = 0;
		else if (o==AND)
			o = AMPER;
		else
			o = STAR;
		goto oponst;

	case LPARN:
		if (andflg) {
			o = symbol();
			if (o==RPARN)
				o = MCALL;
			else {
				peeksym = o;
				o = CALL;
				andflg = 0;
			}
		}
		goto oponst;

	case RBRACK:
	case RPARN:
		if (!andflg)
			goto syntax;
		goto oponst;

	case DOT:
	case ARROW:
		mosflg = FMOS;
		break;

	case ASSIGN:
		if (andflg==0 && PLUS<=*op && *op<=EXOR) {
			o = *op-- + ASPLUS - PLUS;
			pp--;
			goto oponst;
		}
		break;

	}
	/* binaries */
	if (andflg==0)
		goto syntax;
	andflg = 0;

oponst:
	p = (opdope[o]>>9) & 037;
opon1:
	if (o==COLON && op[0]==COLON && op[-1]==QUEST) {
		build(*op--);
		build(*op--);
		pp -= 2;
	}
	ps = *pp;
	if (p>ps || (p==ps && (opdope[o]&RASSOC)!=0)) {
		switch (o) {

		case INCAFT:
		case DECAFT:
			p = 37;
			break;
		case LPARN:
		case LBRACK:
		case CALL:
			p = 04;
		}
		if (initflg) {
			if ((o==COMMA && *op!=LPARN && *op!=CALL)
			 || (o==COLON && *op!=QUEST)) {
				p = 00;
				goto opon1;
			}
		}
		if (op >= &opst[SSIZE-1]) {
			error("expression overflow");
			exit(1);
		}
		*++op = o;
		*++pp = p;
		goto advanc;
	}
	--pp;
	os = *op--;
	if (andflg==0 && p>5 &&
	    ((opdope[o]&BINARY)==0 || (o>=INCBEF&&o<=DECAFT)) &&
	    opdope[os]&BINARY)
		goto syntax;
	switch (os) {

	case SEOF:
		peeksym = o;
		build(0);		/* flush conversions */
		if (eflag)
			endtree(svtree);
		return(*--cp);

	case COMMA:
		if (*op != CALL)
			os = SEQNC;
		break;

	case CALL:
		if (o!=RPARN)
			goto syntax;
		build(os);
		goto advanc;

	case MCALL:
		*cp++ = block(NULLOP, INT, (int *)NULL,
		  (union str *)NULL, TNULL, TNULL);
		os = CALL;
		break;

	case INCBEF:
	case INCAFT:
	case DECBEF:
	case DECAFT:
		*cp++ = cblock(1);
		break;

	case LPARN:
		if (o!=RPARN)
			goto syntax;
		goto advanc;

	case LBRACK:
		if (o!=RBRACK)
			goto syntax;
		build(LBRACK);
		goto advanc;
	}
	build(os);
	goto opon1;

syntax:
	error("Expression syntax");
	errflush(o);
	if (eflag)
		endtree(svtree);
	return((union tree *) &garbage);
}

union tree *
xprtype()
{
	struct nmlist typer, absname;
	int sc;
	register union tree **scp;

	scp = cp;
	sc = DEFXTRN;		/* will cause error if class mentioned */
	getkeywords(&sc, &typer);
	absname.hclass = 0;
	absname.hblklev = blklev;
	absname.hsubsp = NULL;
	absname.hstrp = NULL;
	absname.htype = 0;
	decl1(sc, &typer, 0, &absname);
	cp = scp;
	return(block(ETYPE, absname.htype, absname.hsubsp,
	   absname.hstrp, TNULL, TNULL));
}

char *
copnum(len)
{
	register char *s1;

	s1 = Tblock((len+LNCPW-1) & ~(LNCPW-1));
	strcpy(s1, numbuf);
	return(s1);
}

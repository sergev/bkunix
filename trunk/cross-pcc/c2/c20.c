/*
 *	 C object code improver
 *
 *	@(#)	c20.c	2.0	(2.11BSD)	1996/6/12
 */

#include "c2.h"

struct optab optab[] = {
	{ "br",		JBR, },
	{ "beq",	CBR | JEQ<<8, },
	{ "bne",	CBR | JNE<<8, },
	{ "ble",	CBR | JLE<<8, },
	{ "bge",	CBR | JGE<<8, },
	{ "blt",	CBR | JLT<<8, },
	{ "bgt",	CBR | JGT<<8, },
	{ "blo",	CBR | JLO<<8, },
	{ "bhi",	CBR | JHI<<8, },
	{ "blos",	CBR | JLOS<<8, },
	{ "bhis",	CBR | JHIS<<8, },
	{ "bpl",	CBR | JPL << 8, },
	{ "bmi",	CBR | JMI << 8, },
	{ "jmp",	JMP, },
	{ ".globl",	EROU, },
	{ "mov",	MOV, },
	{ "clr",	CLR, },
	{ "com",	COM, },
	{ "inc",	INC, },
	{ "dec",	DEC, },
	{ "neg",	NEG, },
	{ "tst",	TST, },
	{ "asr",	ASR, },
	{ "asl",	ASL, },
	{ "sxt",	SXT, },
	{ "cmp",	CMP, },
	{ "add",	ADD, },
	{ "sub",	SUB, },
	{ "bit",	BIT, },
	{ "bic",	BIC, },
	{ "bis",	BIS, },
	{ "mul",	MUL, },
	{ "div",	DIV, },
	{ "ash",	ASH, },
	{ "xor",	XOR, },
	{ ".text",	TEXT, },
	{ ".data",	DATA, },
	{ ".bss",	BSS, },
	{ ".even",	EVEN, },
	{ "movf",	MOVF, },
	{ "movof",	MOVOF, },
	{ "movfo",	MOVFO, },
	{ "addf",	ADDF, },
	{ "subf",	SUBF, },
	{ "divf",	DIVF, },
	{ "mulf",	MULF, },
	{ "clrf",	CLRF, },
	{ "cmpf",	CMPF, },
	{ "negf",	NEGF, },
	{ "tstf",	TSTF, },
	{ "cfcc",	CFCC, },
	{ "sob",	SOB, },
	{ "jsr",	JSR, },
	{ "swab",	SWAB, },
	{ ".end",	END, },
	{ 0,		0, },
};

char revbr[] = {JNE, JEQ, JGT, JLT, JGE, JLE, JHIS, JLOS, JHI, JLO, JMI, JPL};
int	isn	= 20000;
int	lastseg	= -1;

#define	NSTK	5000

void	opsetup();
int	input();
void	refcount();
void	iterate();
void	comjump();
void	output();
int	getline();
int	getnum();
int	oplook();
void	reducelit();
void	xjump();
void	backjmp();

int main(argc, argv)
char **argv;
{
	register int niter, maxiter, isend;
	int nflag;
	char	stspace[NSTK],
		buf1[BUFSIZ],
		buf2[BUFSIZ],
		stderrbuf[80];

	if (argc>1 && argv[1][0]=='+') {
		argc--;
		argv++;
		debug++;
	}
	nflag = 0;
	if (argc>1 && argv[1][0]=='-' && argv[1][1]=='n') {
		argc--;
		argv++;
		nflag++;
	}
	setbuffer(stderr, stderrbuf, sizeof (stderrbuf));

	if (argc>1) {
		if (freopen(argv[1], "r", stdin) == NULL) {
			fprintf(stderr, "C2: can't find %s\n", argv[1]);
			exit(1);
		}
	}
	setbuf(stdin,buf1);		/* sbrk problems */
	if (argc>2) {
		if (freopen(argv[2], "w", stdout) == NULL) {
			fprintf(stderr, "C2: can't create %s\n", argv[2]);
			exit(1);
		}
	}
	setbuf(stdout,buf2);		/* sbrk problems */
	lasta = firstr = lastr = sbrk(sizeof(char *));
	maxiter = 0;
	opsetup();
	do {
		alasta = stspace;
		alastr = &stspace[NSTK];
		isend = input();
		movedat();
		niter = 0;
		do {
			refcount();
			do {
				iterate();
				clearreg();
				niter++;
			} while (nchange);
			comjump();
			rmove();
		} while (nchange || jumpsw());
		addsob();
		output();
		if (niter > maxiter)
			maxiter = niter;
		lasta = firstr;
	} while (isend);
	if (nflag) {
		fprintf(stderr, "%d iterations\t\t\t", maxiter);
		fprintf(stderr, "%d jumps to jumps\n", nbrbr);
		fprintf(stderr, "%d inst. after jumps\t\t", iaftbr);
		fprintf(stderr, "%d jumps to .+2\n", njp1);
		fprintf(stderr, "%d redundant labels\t\t", nrlab);
		fprintf(stderr, "%d cross-jumps\n", nxjump);
		fprintf(stderr, "%d code motions\t\t\t", ncmot);
		fprintf(stderr, "%d branches reversed\n", nrevbr);
		fprintf(stderr, "%d redundant moves\t\t", redunm);
		fprintf(stderr, "%d simplified addresses\n", nsaddr);
		fprintf(stderr, "%d loops inverted\t\t", loopiv);
		fprintf(stderr, "%d redundant jumps\n", nredunj);
		fprintf(stderr, "%d common seqs before jmp's\t", ncomj);
		fprintf(stderr, "%d skips over jumps\n", nskip);
		fprintf(stderr, "%d sob's added\t\t\t", nsob);
		fprintf(stderr, "%d redundant tst's\n", nrtst);
		fprintf(stderr, "%d literals eliminated\t\t", nlit);
		fprintf(stderr, "%dK core\n", (((int)lastr+01777)>>10)&077);
	}
	exit(0);
}

int input()
{
	register struct node *p, *lastp;
	register int oper;

	lastp = &first;
	for (;;) {
		oper = getline();
		switch (oper&0377) {

		case LABEL:
			p = (struct node *)alloc(sizeof first);
			if (line[0] == 'L') {
				p->op = LABEL;
				p->subop = 0;
				p->labno = getnum(line+1);
				p->code = 0;
			} else {
				p->op = DLABEL;
				p->subop = 0;
				p->labno = 0;
				p->code = copy(1, line);
			}
			break;

		case JBR:
		case CBR:
		case JMP:
		case JSW:
			p = (struct node *)alloc(sizeof first);
			p->op = oper&0377;
			p->subop = oper>>8;
			if (*curlp=='L' && (p->labno = getnum(curlp+1)))
				p->code = 0;
			else {
				p->labno = 0;
				p->code = copy(1, curlp);
			}
			break;

		default:
			p = (struct node *)alloc(sizeof first);
			p->op = oper&0377;
			p->subop = oper>>8;
			p->labno = 0;
			p->code = copy(1, curlp);
			break;

		}
		p->forw = 0;
		p->back = lastp;
		lastp->forw = p;
		lastp = p;
		p->ref = 0;
		if (oper==EROU)
			return(1);
		if (oper==END)
			return(0);
	}
}

int getline()
{
	register char *lp;
	register int c;

	lp = line;
	while ((c = getchar())==' ' || c=='\t')
		;
	do {
		if (c==':') {
			*lp++ = 0;
			return(LABEL);
		}
		if (c=='\n') {
			*lp++ = 0;
			return(oplook());
		}
		if (lp >= &line[LSIZE-2]) {
			fprintf(stderr, "C2: Sorry, input line too long\n");
			exit(1);
		}
		*lp++ = c;
	} while ((c = getchar()) != EOF);
	*lp++ = 0;
	return(END);
}

int getnum(ap)
char *ap;
{
	register char *p;
	register int n, c;

	p = ap;
	n = 0;
	while ((c = *p++) >= '0' && c <= '9')
		n = n*10 + c - '0';
	if (*--p != 0)
		return(0);
	return(n);
}

void output()
{
	register struct node *t;
	register struct optab *oper;
	register int byte;

	t = &first;
	while ((t = t->forw) != 0) switch (t->op) {

	case END:
		return;

	case LABEL:
		printf("L%d:", t->labno);
		continue;

	case DLABEL:
		printf("%s:", t->code);
		continue;

	case TEXT:
	case DATA:
	case BSS:
		lastseg = t->op;

	default:
		if ((byte = t->subop) == BYTE)
			t->subop = 0;
		for (oper = optab; oper->opstring!=0; oper++)
			if ((oper->opcode&0377) == t->op
			 && (oper->opcode>>8) == t->subop) {
				printf("%s", oper->opstring);
				if (byte==BYTE)
					printf("b");
				break;
			}
		if (t->code) {
			reducelit(t);
			printf("\t%s\n", t->code);
		} else if (t->op==JBR || t->op==CBR)
			printf("\tL%d\n", t->labno);
		else
			printf("\n");
		continue;

	case JSW:
		printf("L%d\n", t->labno);
		continue;

	case SOB:
		printf("sob	%s", t->code);
		if (t->labno)
			printf(",L%d", t->labno);
		printf("\n");
		continue;

	case 0:
		if (t->code)
			printf("%s", t->code);
		printf("\n");
		continue;
	}
}

/*
 * Notice addresses of the form
 * $xx,xx(r)
 * and replace them with (pc),xx(r)
 *     -- Thanx and a tip of the Hatlo hat to Bliss-11.
 */
void reducelit(at)
struct node *at;
{
	register char *c1, *c2;
	char *c2s;
	register struct node *t;

	t = at;
	if (*t->code != '$')
		return;
	c1 = t->code;
	while (*c1 != ',')
		if (*c1++ == '\0')
			return;
	c2s = c1;
	c1++;
	if (*c1=='*')
		c1++;
	c2 = t->code+1;
	while (*c1++ == *c2++);
	if (*--c1!='(' || *--c2!=',')
		return;
	t->code = copy(2, "(pc)", c2s);
	nlit++;
}

char *
copy(na, ap)
char *ap;
{
	register char *p, *np;
	char *onp;
	register int n;

	p = ap;
	n = 0;
	if (*p==0)
		return(0);
	do
		n++;
	while (*p++);
	if (na>1) {
		p = (&ap)[1];
		while (*p++)
			n++;
	}
	onp = np = alloc(n);
	p = ap;
	while ((*np++ = *p++) != 0)
		;
	if (na>1) {
		p = (&ap)[1];
		np--;
		while ((*np++ = *p++) != 0);
	}
	return(onp);
}

void opsetup()
{
	register struct optab *optp, **ophp;
	register char *p;

	for (optp = optab; (p = optp->opstring) != 0; optp++) {
		ophp = &ophash[(((p[0]<<3)+(p[1]<<1)+p[2])&077777) % OPHS];
		while (*ophp++)
			if (ophp > &ophash[OPHS])
				ophp = ophash;
		*--ophp = optp;
	}
}

int oplook()
{
	register struct optab *optp;
	register char *lp, *np;
	static char tmpop[32];
	struct optab **ophp;

	if (line[0]=='\0') {
		curlp = line;
		return(0);
	}
	np = tmpop;
	for (lp = line; *lp && *lp!=' ' && *lp!='\t';)
		*np++ = *lp++;
	*np++ = 0;
	while (*lp=='\t' || *lp==' ')
		lp++;
	curlp = lp;
	ophp = &ophash[(((tmpop[0]<<3)+(tmpop[1]<<1)+tmpop[2])&077777) % OPHS];
	while ((optp = *ophp) != 0) {
		np = optp->opstring;
		lp = tmpop;
		while (*lp == *np++)
			if (*lp++ == 0)
				return(optp->opcode);
		if (*lp++=='b' && *lp++==0 && *--np==0)
			return(optp->opcode + (BYTE<<8));
		ophp++;
		if (ophp >= &ophash[OPHS])
			ophp = ophash;
	}
	if (line[0]=='L' && line[1]!='F') {
		lp = &line[1];
		while (*lp)
			if (*lp<'0' || *lp++>'9')
				return(0);
		curlp = line;
		return(JSW);
	}
	curlp = line;
	return(0);
}

void refcount()
{
	register struct node *p, *lp;
	static struct node *labhash[LABHS];
	register struct node **hp, *tp;

	for (hp = labhash; hp < &labhash[LABHS];)
		*hp++ = 0;
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL) {
			labhash[p->labno % LABHS] = p;
			p->refc = 0;
		}
	for (p = first.forw; p!=0; p = p->forw) {
		if (p->op==JBR || p->op==CBR || p->op==JSW) {
			p->ref = 0;
			lp = labhash[p->labno % LABHS];
			if (lp==0 || p->labno!=lp->labno)
			for (lp = first.forw; lp!=0; lp = lp->forw) {
				if (lp->op==LABEL && p->labno==lp->labno)
					break;
			}
			if (lp) {
				tp = nonlab(lp)->back;
				if (tp!=lp) {
					p->labno = tp->labno;
					lp = tp;
				}
				p->ref = lp;
				lp->refc++;
			}
		}
	}
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL && p->refc==0
		 && (lp = nonlab(p))->op && lp->op!=JSW)
			decref(p);
}

void iterate()
{
	register struct node *p, *rp, *p1;

	nchange = 0;
	for (p = first.forw; p!=0; p = p->forw) {
		CHECK(0);
		if ((p->op==JBR||p->op==CBR||p->op==JSW) && p->ref) {
			rp = nonlab(p->ref);
			if (rp->op==JBR && rp->labno && p->labno!=rp->labno) {
				nbrbr++;
				p->labno = rp->labno;
				decref(p->ref);
				rp->ref->refc++;
				p->ref = rp->ref;
				CHECK(1);
				nchange++;
			}
		}
		if (p->op==CBR && (p1 = p->forw)->op==JBR) {
			rp = p->ref;
			do
				rp = rp->back;
			while (rp->op==LABEL);
			if (rp==p1) {
				decref(p->ref);
				p->ref = p1->ref;
				p->labno = p1->labno;
				p1->forw->back = p;
				p->forw = p1->forw;
				p->subop = revbr [(int) p->subop];
				nchange++;
				CHECK(2);
				nskip++;
			}
		}
		if (p->op==JBR || p->op==JMP) {
			while (p->forw && p->forw->op!=LABEL
				&& p->forw->op!=DLABEL
				&& p->forw->op!=EROU && p->forw->op!=END
/* v7.orig			&& p->forw->op!=0 && p->forw->op!=DATA) { */
/*
 * next 2 lines fixes bug when there are unitialized
 * static variables in the middle of a routine
 */
				&& p->forw->op!=0 && p->forw->op!=DATA
				&& p->forw->op!=BSS) {
				nchange++;
				iaftbr++;
				if (p->forw->ref)
					decref(p->forw->ref);
				p->forw = p->forw->forw;
				p->forw->back = p;
				CHECK(3);
			}
			rp = p->forw;
			while (rp && rp->op==LABEL) {
				if (p->ref == rp) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
					p = p->back;
					decref(rp);
					nchange++;
					CHECK(4);
					njp1++;
					break;
				}
				rp = rp->forw;
			}
		}
		if (p->op==JBR || p->op==JMP) {
			xjump(p);
			p = codemove(p);
		}
	}
}

void xjump(p1)
register struct node *p1;
{
	register struct node *p2, *p3;

	if ((p2 = p1->ref)==0)
		return;
	for (;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		while ((p2 = p2->back) && p2->op==LABEL);
		if (!equop(p1, p2) || p1==p2)
			return;
		p3 = insertl(p2);
		p1->op = JBR;
		p1->subop = 0;
		p1->ref = p3;
		p1->labno = p3->labno;
		p1->code = 0;
		nxjump++;
		CHECK(5);
		nchange++;
	}
}

struct node *
insertl(oldp)
register struct node *oldp;
{
	register struct node *lp;

	if (oldp->op == LABEL) {
		oldp->refc++;
		return(oldp);
	}
	if (oldp->back->op == LABEL) {
		oldp = oldp->back;
		oldp->refc++;
		return(oldp);
	}
	lp = (struct node *)alloc(sizeof first);
	lp->op = LABEL;
	lp->subop = 0;
	lp->labno = isn++;
	lp->ref = 0;
	lp->code = 0;
	lp->refc = 1;
	lp->back = oldp->back;
	lp->forw = oldp;
	oldp->back->forw = lp;
	oldp->back = lp;
	CHECK(6);
	return(lp);
}

struct node *
codemove(p)
struct node *p;
{
	register struct node *p1, *p2, *p3;
	struct node *t, *tl;
	int n;

	p1 = p;
	if (p1->op!=JBR || (p2 = p1->ref)==0)
		return(p1);
	while (p2->op == LABEL)
		if ((p2 = p2->back) == 0)
			return(p1);
	if (p2->op!=JBR && p2->op!=JMP)
		goto ivloop;
	if (p1==p2)
		return(p1);
	p2 = p2->forw;
	p3 = p1->ref;
	while (p3) {
		if (p3->op==JBR || p3->op==JMP) {
			if (p1==p3 || p1->forw==p3 || p1->back==p3)
				return(p1);
			ncmot++;
			nchange++;
			CHECK(70);
			p1->back->forw = p2;
			p1->forw->back = p3;
			p2->back->forw = p3->forw;
			p3->forw->back = p2->back;
			p2->back = p1->back;
			p3->forw = p1->forw;
			decref(p1->ref);
			CHECK(7);
			return(p2);
		} else
			p3 = p3->forw;
	}
	return(p1);
ivloop:
	if (p1->forw->op!=LABEL)
		return(p1);
	p3 = p2 = p2->forw;
	n = 16;
	do {
		if ((p3 = p3->forw) == 0 || p3==p1 || --n==0)
			return(p1);
	} while (p3->op!=CBR || p3->labno!=p1->forw->labno);
	do
		if ((p1 = p1->back) == 0)
			return(p);
	while (p1!=p3);
	p1 = p;
	tl = insertl(p1);
	p3->subop = revbr [(int) p3->subop];
	decref(p3->ref);
	p2->back->forw = p1;
	p3->forw->back = p1;
	p1->back->forw = p2;
	p1->forw->back = p3;
	t = p1->back;
	p1->back = p2->back;
	p2->back = t;
	t = p1->forw;
	p1->forw = p3->forw;
	p3->forw = t;
	p2 = insertl(p1->forw);
	p3->labno = p2->labno;
	p3->ref = p2;
	decref(tl);
	if (tl->refc<=0)
		nrlab--;
	loopiv++;
	nchange++;
	CHECK(8);
	return(p3);
}

void comjump()
{
	register struct node *p1, *p2, *p3;

	for (p1 = first.forw; p1!=0; p1 = p1->forw)
		if (p1->op==JBR && (p2 = p1->ref) && p2->refc > 1)
			for (p3 = p1->forw; p3!=0; p3 = p3->forw)
				if (p3->op==JBR && p3->ref == p2)
					backjmp(p1, p3);
}

void backjmp(ap1, ap2)
struct node *ap1, *ap2;
{
	register struct node *p1, *p2, *p3;

	p1 = ap1;
	p2 = ap2;
	for(;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		p2 = p2->back;
		if (equop(p1, p2)) {
			p3 = insertl(p1);
			p2->back->forw = p2->forw;
			p2->forw->back = p2->back;
			p2 = p2->forw;
			decref(p2->ref);
			p2->labno = p3->labno;
			p2->ref = p3;
			nchange++;
			ncomj++;
			CHECK(9);
		} else
			return;
	}
}

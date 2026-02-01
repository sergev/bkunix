/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _PASS1_
#define	_PASS1_

#include "macdefs.h"
#include "manifest.h"

/*
 * Symbol table definition.
 *
 * Colliding entries are moved down with a standard
 * probe (no quadratic rehash here) and moved back when
 * entries are cleared.
 */
struct	symtab {
#ifndef FLEXNAMES
	char	sname[NCHNAM];
#else
	char	*sname;
#endif
	struct	symtab *snext;	/* link to other symbols in the same scope */
	TWORD	stype;		/* type word */
	char	sclass;		/* storage class */
	char	slevel;		/* scope level */
	char	sflags;		/* flags, see below */
	OFFSZ	offset;		/* offset or value */
	short	dimoff;		/* offset into the dimension table */
	short	sizoff;		/* offset into the size table */
	int	suse;		/* line number of last use of the variable */
};

/*
 * Storage classes
 */
#define SNULL		0		/* initial value */
#define AUTO		1		/* automatic (on stack) */
#define EXTERN		2		/* external reference */
#define STATIC		3		/* static scope */
#define REGISTER	4		/* register requested */
#define EXTDEF		5		/* external definition */
#define LABEL		6		/* label definition */
#define ULABEL		7		/* undefined label reference */
#define MOS		8		/* member of structure */
#define PARAM		9		/* parameter */
#define STNAME		10		/* structure name */
#define MOU		11		/* member of union */
#define UNAME		12		/* union name */
#define TYPEDEF		13		/* typedef name */
#define FORTRAN		14		/* fortran function */
#define ENAME		15		/* enumeration name */
#define MOE		16		/* member of enumeration */
#define UFORTRAN 	17		/* undefined fortran reference */
#define USTATIC		18		/* undefined static reference */

/* field size is ORed in */
#define FIELD		0100
#define FLDSIZ		077
#ifndef BUG1
extern	char *scnames(int);
#endif

/*
 * Symbol table flags
 */
#define SMOS		01		/* member of structure */
#define SHIDDEN		02		/* hidden in current scope */
#define SHIDES		04		/* hides symbol in outer scope */
#define SSET		010		/* symbol assigned to */
#define SREF		020		/* symbol referenced */
#define SNONUNIQ	040		/* non-unique structure member */
#define STAG		0100		/* structure tag name */

/*
 * Location counters
 */
#define PROG		0		/* program segment */
#define DATA		1		/* data segment */
#define ADATA		2		/* array data segment */
#define STRNG		3		/* string data segment */
#define ISTRNG		4		/* initialized string segment */
#define STAB		5		/* symbol table segment */


#ifndef ONEPASS
#include "ndu.h"
#endif


#ifndef FIXDEF
#define FIXDEF(p)
#endif
#ifndef FIXARG
#define FIXARG(p)
#endif
#ifndef FIXSTRUCT
#define FIXSTRUCT(a,b)
#endif

	/* alignment of initialized quantities */
#ifndef AL_INIT
#define	AL_INIT ALINT
#endif

/*
 * External definitions
 */
struct sw {		/* switch table */
	CONSZ	sval;	/* case value */
	int	slab;	/* associated label */
};
extern	struct sw swtab[];
extern	struct sw *swp;
extern	int swx;

extern	int ftnno;
extern	int blevel;
extern	int instruct, stwart;

extern	int lineno, nerrors;

extern	CONSZ lastcon;
extern	float fcon;
extern	double dcon;

extern	char ftitle[];
#ifndef	LINT
extern	char ititle[];
#endif
extern	struct symtab stab[];
extern	int curftn;
extern	int curclass;
extern	int curdim;
extern	OFFSZ dimtab[];
extern	OFFSZ paramstk[];
extern	int paramno;
extern	OFFSZ autooff, argoff, strucoff;
extern	int regvar;
extern	int minrvar;
extern	int brkflag;
typedef union {
	int intval;
	NODE * nodep;
	} YYSTYPE;
extern	YYSTYPE yylval;
extern	char yytext[];

extern	int strflg;

extern	OFFSZ inoff;

extern	int reached;

/* tunnel to buildtree for name id's */
extern	int idname;

extern	NODE node[];
extern	NODE *lastfree;

extern	int cflag, hflag, pflag;

/* various labels */
extern	int brklab;
extern	int contlab;
extern	int flostat;
extern	int retlab;
extern	int retstat;
extern	int asavbc[], *psavbc;

/* declarations of various functions */
extern	NODE	*buildtree(int, NODE *, NODE *);
extern	NODE	*block(int, NODE *, NODE *, TWORD, int, int);
extern	NODE	*optim(NODE *);
extern	NODE	*bdty(int, NODE *, int);
extern	NODE	*mkty(unsigned int, int, int);
extern	NODE	*rstruct(int, int);
extern	NODE	*dclstruct(int);
extern	NODE	*getstr(void);
extern	NODE	*tymerge(NODE *, NODE *);
extern	NODE	*stref(NODE *);
extern	NODE	*offcon(OFFSZ, TWORD, int, int);
extern	NODE	*bcon(int);
extern	NODE	*bpsize(NODE *);
extern	NODE	*convert(NODE *, int);
extern	NODE	*pconvert(NODE *);
extern	NODE	*oconvert(NODE *);
extern	NODE	*ptmatch(NODE *);
extern	NODE	*tymatch(NODE *);
extern	NODE	*makety(NODE *, TWORD, int, int);
extern	NODE	*doszof(NODE *);
extern	NODE	*talloc(void);
extern	NODE	*fixargs(NODE *);
extern	NODE	*clocal(NODE *);
extern	OFFSZ	tsize(TWORD, int, int);
extern	OFFSZ	psize(NODE *);
extern	TWORD	types(TWORD, TWORD, TWORD);
extern	double	atof(const char *);
extern	TWORD	ctype(TWORD);
extern	int	lookup(const char *, int);
extern	char	*exname(char *);
extern	char	*exdcon(int);

void	cerror(const char *, ...);
void	uerror(const char *, ...);
void	werror(const char *, ...);
void	tfree(NODE *);

/* PCC backend / codegen interface (pass1) */
void	where(int);
int	branch(int);
int	defalign(int);
int	locctr(int);
void	deflab(int);
int	getlab(void);
void	efcode(void);
void	psline(void);
void	ecomp(NODE *);
void	bccode(void);
void	plcstab(int);
void	yyerror(const char *);
int	yyparse(void);
void	yyaccpt(void);
int	p2init(int, char *[]);
void	lxinit(void);
void	tinit(void);
void	mkdope(void);
void	pfstab(const char *);
void	ejobcode(int);
void	ftnend(void);
NODE	*defid(NODE *, int);
void	ecode(NODE *);
NODE	*addroreg(NODE *);
void	p2tree(NODE *);
void	p2compile(NODE *);
void	tcheck(void);
void	fixarg(struct symtab *);
int	cendarg(void);
void	bfcode(OFFSZ [], int);
void	p2bend(void);
void	p2bbeg(OFFSZ, int);
int	talign(unsigned int, int);
void	outstruct(int, int);
void	beginit(int);
void	endinit(void);
void	doinit(NODE *);
void	ilbrace(void);
void	irbrace(void);
void	nidcl(NODE *);
int	uclass(int);
void	clearst(int);
void	prcstab(int);
void	defnam(struct symtab *);
void	pstab(const void *, int);
unsigned int	fldal(TWORD);
void	vfdzero(int);
void	zecode(OFFSZ);
void	instk(int, TWORD, int, int, OFFSZ);
int	mainp1(int, char *[]);
void	lxstr(int);
void	lxtitle(void);
int	lxres(void);
int	isitfloat(const char *);
void	putbyte(int);
void	bycode(int, int);
void	makeheap(struct sw *, int, int);
void	walkheap(int, int);
int	hselect(int);
int	bstruct(int, int);
void	incode(NODE *, int);
void	gotscal(void);
void	fincode(double, int);
void	cinit(NODE *, int);
int	icons(NODE *);
TWORD	moditype(TWORD);
void	rbusy(int, TWORD);
int	noinit(void);
int	commdec(int);
int	fldty(struct symtab *);

/* common.c (comm1.c) */
extern	OFFSZ	caloff(void);
extern	void	fwalk(NODE *, void (*)(NODE *, int, int *, int *), int);
extern	void	walkf(NODE *, void (*)(NODE *));
extern	void	mkdope(void);
extern	void	tprint(TWORD);
extern	void	eprint(NODE *, int, int *, int *);

/* trees.c, allo.c, optim.c cross-refs */
extern	int	conval(NODE *, int, NODE *);
extern	NODE	*econvert(NODE *);
extern	void	allo0(void);
extern	OFFSZ	freetemp(int);
extern	int	freereg(NODE *, int);
extern	int	usable(NODE *, int, int);
extern	void	reclaim(NODE *, int, int);
extern	void	allchk(void);
extern	int	ispow2(CONSZ);
extern	int	notlval(NODE *);
extern	void	delay(NODE *);
extern	void	tyreduce(NODE *);
extern	void	fixtype(NODE *, int);
extern	TWORD	fixclass(int, TWORD);
extern	void	moedef(int);
extern	void	ftnarg(int);
extern	void	dclargs(void);
extern	int	cisreg(TWORD);
extern	void	aobeg(void);
extern	void	aocode(struct symtab *);
extern	void	aoend(void);
extern	void	unhide(struct symtab *);
extern	int	falloc(struct symtab *, int, int, NODE *);
extern	void	genswitch(struct sw *, int);
extern	int	andable(NODE *);
extern	NODE	*lineid(NODE *, char *);
extern	int	callreg(NODE *);
extern	NODE	*shtemp(NODE *);
extern	void	outstab(struct symtab *);
extern	void	psave(OFFSZ);
extern	OFFSZ	oalloc(struct symtab *, OFFSZ *);
extern	struct symtab	*mknonuniq(int *);
extern	int	hide(struct symtab *);
extern	void	dstash(OFFSZ);
extern	int	opact(NODE *);
extern	void	chkpun(NODE *);
extern	int	chkstr(int, int, TWORD);

#define checkst(x)

#ifndef CHARCAST
/* to make character constants into character connstants */
/* this is a macro to defend against cross-compilers, etc. */
#define CHARCAST(x) (char)(x)
#endif
#endif

/*
 * Flags used in structures/unions
 */
#define SEENAME		01
#define INSTRUCT	02
#define INUNION		04
#define FUNNYNAME	010
#define TAGNAME		020

/*
 * Flags used in the (elementary) flow analysis ...
 */
#define FBRK		02
#define FCONT		04
#define FDEF		010
#define FLOOP		020

/*
 * Flags used for return status
 */
#define RETVAL		1
#define NRETVAL		2

#define NONAME		040000		/* marks constant w/o name field */
#define NOOFFSET	(-10201)	/* mark an offset which is undefined */

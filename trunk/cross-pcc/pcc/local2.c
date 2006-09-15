# include "pass2.h"
extern	int	fltused;
#define	putstr(s)	fputs((s),stdout)

/* a lot of the machine dependent parts of the second pass */

# define BITMASK(n) ((1L<<n)-1)
where(c) {
	fprintf(stderr, "%s, line %d: ", filename, lineno);
	}

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "/	line %d, file %s\n", l, fn );
	}

eobl2(){
	OFFSZ spoff;	/* offset from stack pointer */

	spoff = maxoff;
	if( spoff >= AUTOINIT ) spoff -= AUTOINIT;
	spoff /= SZCHAR;
	SETOFF(spoff,2);
	printf( "	.F%d = %ld.\n", ftnno, spoff );
	if( fltused ) {
		fltused = 0;
		printf( "	.globl	fltused\n" );
		}
	}

struct hoptab { int opmask; char * opstring; } ioptab[]= {

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG OR,	"bis",
	ASG AND,	"bic",
	ASG ER,	"xor",
	ASG MUL, "mul",
	ASG DIV, "div",
	ASG MOD, "div",
	ASG LS,	"asl",
	ASG RS,	"asr",

	-1, ""    };

hopcode( f, o ){
	/* output the appropriate string from the above table */

	register struct hoptab *q;

	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			printf( "%s", q->opstring );
			if( f == 'F' ) printf( "f" );
			return;
			}
		}
	cerror( "no hoptab for %s", opst[o] );
	}

char *
rnames[]= {  /* keyed to register number tokens */

	"r0", "r1",
	"r2", "r3", "r4",
	"r5", "sp", "pc",

	"fr0", "fr1", "fr2", "fr3",
	"fr4", "fr5",	/* not accumulators - used for temps */
	};

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG, SAREG|STAREG,	/* use as scratch if not reg var */
	SAREG, SAREG, SAREG,

	SBREG|STBREG, SBREG|STBREG, SBREG|STBREG, SBREG|STBREG,
	SBREG, SBREG,
	};

tlen(p) NODE *p; 
{
	switch(p->in.type) {
		case CHAR:
		case UCHAR:
			return(1);
			
		case LONG:
		case ULONG:
		case FLOAT:
			return(4);

		case DOUBLE:
			return(8);
			
		default:
			return(2);
		}
	}
NODE *brnode;
int brcase;

int toff = 0; /* number of stack locations used for args */

zzzcode( p, c ) NODE *p; {
	register m;
	switch( c ){

	case 'B':	/* output b if type is byte */
		if( p->in.type == CHAR || p->in.type == UCHAR ) printf( "b" );
		return;

	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	clr	%s\n", rnames[getlr( p, '1' )->tn.rval] );
		if( p->in.type == LONG || p->in.type == ULONG )
			printf( "	clr	%s\n", rnames[getlr(p, '1')->tn.rval + 1] );
		deflab( m );
		return;

	case 'I':
	case 'F':
		cbgen( p->in.op, p->bn.label, c );
		return;

	case 'A':
	case 'C':
		/* logical operators for longs
		   defer comparisons until branch occurs */

		brnode = tcopy( p );
		brcase = c;
		return;

	case 'H':  /* fix up unsigned shifts */
		{	register NODE *q;
			register r, l;
			TWORD t;

			if( p->in.op == ASG LS ) return;
			if( p->in.op != ASG RS ) cerror( "ZH bad" );
			if( p->in.left->in.op != REG ) cerror( "SH left bad" );

			r = p->in.left->tn.rval;
			t = p->in.left->in.type;
			l = (t==LONG || t == ULONG );

			if( t != UNSIGNED && t != UCHAR && t != ULONG ) return;  /* signed is ok */

			/* there are three cases:  right side is a constant,
				and has the shift value; right side is
				a temporary reg, and has the - shift value,
				and right side is something else: A1 has the
				- shift value then */

			/* in the case where the value is known (rhs a constant),
				the mask is just computed and put out... */

			if( p->in.right->in.op == ICON ){
				int s;
				s = p->in.right->tn.lval;
				if( l ){
					if( s >= 16 ){
						printf( "	clr	r%d\n", r );
						s -= 16;
						++r;
						}
					}
				if( s >= 16 ) printf( "	clr	r%d\n", r );
				else {
					m = 0100000;
					m >>= s;  /* sign extends... */
					m <<= 1;
					printf( "	bic	$%o,r%d\n", m, r );
					}
				return;
				}

			/* general case */

			if( istnode( p->in.right ) ) q = p->in.right;
			else q = getlr( p, '1' );  /* where -shift is stored */

			/* first, we store the shifted value on the stack */
			printf( "	mov	r%d,-(sp)\n", r );
			if( l ) printf( "	mov	r%d,-(sp)\n", r+1 );

			/* now, make a mask */

			printf( "	mov	$100000,r%d\n", r );
			if( l ) printf( "	clr	r%d\n", r+1 );
			
			/* shift (arithmetically ) */
			if( l ) expand( q, RNOP, "	ashc	AR" );
			else expand( q, RNOP, "	ash	AR" );
			printf( ",r%d\n", r );

			if( l ) printf( "	ashc	$1,r%d\n", r );
			else printf( "	asl	r%d\n", r );

			/* now, we have a mask: use it to clear sp, and reload */

			if( l ){
				printf( "\tbic\tr%d,(sp)\n\tmov\t(sp)+,r%d\n", r+1, r+1 );
				}
			printf( "\tbic\tr%d,(sp)\n\tmov\t(sp)+,r%d\n", r, r );
			/* whew! */
			return;
			}

	case 'V':
		/* sign extend or not -- register is one less than the
		   left descendent */

		m = p->in.left->tn.rval - 1;

		if( ISUNSIGNED(p->in.type) ){
			printf( "	clr	r%d\n", m );
			}
		else {
			printf( "	sxt	r%d\n", m );
			}
		return;

		/* stack management macros */
	case '-':
		if( toff ++ ) printf( "-" );
		printf( "(sp)" );
		return;

	case '4':
		if( toff == 0 ) ++toff;  /* can't push doubles that way */
		printf( "-(sp)" );
		toff += 4;
		return;

	case '~':
		/* complimented CR */
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		return;

	case 'M':
		/* negated CR */
		p->in.right->tn.lval = -p->in.right->tn.lval;
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = -p->in.right->tn.lval;
		return;

	case 'L':  /* INIT for long constants */
		{
			unsigned hi, lo;
			lo = p->in.left->tn.lval & BITMASK(SZINT);
			hi = ( p->in.left->tn.lval >> SZINT ) & BITMASK(SZINT);
			printf( "	%o; %o\n", hi, lo );
			return;
		}

	case 'T':
		/* Truncate longs for type conversions:
		    LONG|ULONG -> CHAR|UCHAR|INT|UNSIGNED
		   increment offset to second word */

		m = p->in.type;
		p = p->in.left;
		switch( p->in.op ){
		case NAME:
		case OREG:
			p->tn.lval += SZINT/SZCHAR;
			return;
		case REG:
			rfree( p->tn.rval, p->in.type );
			p->tn.rval += 1;
			p->in.type = m;
			rbusy( p->tn.rval, p->in.type );
			return;
		default:
			cerror( "Illegal ZT type conversion" );
			return;

			}

	case 'U':
		/* same as AL for exp under U* */
		if( p->in.left->in.op == UNARY MUL ) {
			adrput( getlr( p->in.left, 'L' ) );
			return;
			}
		cerror( "Illegal ZU" );
		/* NO RETURN */

	case 'W':	/* structure size */
		if( p->in.op == STASG )
			printf( "%d", p->stn.stsize);
		else	cerror( "Not a structure" );
		return;

	case 'S':  /* structure assignment */
		{
			register NODE *l, *r;
			register size, count;

			if( p->in.op == STASG ){
				l = p->in.left;
				r = p->in.right;
				}
			else if( p->in.op == STARG ){  /* store an arg onto the stack */
				r = p->in.left;
				}
			else cerror( "STASG bad" );

			if( r->in.op == ICON ) r->in.op = NAME;
			else if( r->in.op == REG ) r->in.op = OREG;
			else if( r->in.op != OREG ) cerror( "STASG-r" );

			size = p->stn.stsize;
			count = size / 2;

			r->tn.lval += size;
			if( p->in.op == STASG ) l->tn.lval += size;

			while( count-- ){ /* simple load/store loop */
				r->tn.lval -= 2;
				expand( r, FOREFF, "	mov	AR," );
				if( p->in.op == STASG ){
					l->tn.lval -= 2;
					expand( l, FOREFF, "AR\n" );
					}
				else {
					printf( "-(sp)\n" );
					}

				}

			if( r->in.op == NAME ) r->in.op = ICON;
			else if( r->in.op == OREG ) r->in.op = REG;

			}
		break;

	default:
		cerror( "illegal zzzcode" );
		}
	}

rmove( rt, rs, t ) TWORD t; {
	printf( "	%s	%s,%s\n", (t==FLOAT||t==DOUBLE)?"movf":"mov", rnames[rs], rnames[rt] );
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARREG|SNAME|STARNM|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTAREG,	SOREG|SNAME,
	0,	0 };

setregs(){ /* set up temporary registers */
	register i;

	/* use any unused variable registers as scratch registers */
	fregs = maxtreg>=MINRVAR ? maxtreg + 1 : MINRVAR;
	if( xdebug ){
		/* -x changes number of free regs to 2, -xx to 3, etc. */
		if( (xdebug+1) < fregs ) fregs = xdebug+1;
		}
	/* NOTE: for pdp11 fregs <= 4 for float regs */
	if( fregs > 4 ) fregs = 4;
	for( i=MINRVAR; i<=MAXRVAR; i++ )
		rstatus[i] = i<fregs ? SAREG|STAREG : SAREG;
	}

szty(t) TWORD t; { /* size, in words, needed to hold thing of type t */
	/* really is the number of registers to hold type t */
	switch( t ) {

	case LONG:
	case ULONG:
		return( SZLONG/SZINT );

	default:
		return(1);

		}
	}

rewfld( p ) NODE *p; {
	return(1);
	}

callreg(p) NODE *p; {
	return( (p->in.type==DOUBLE||p->in.type==FLOAT) ? FR0 : R0 );
	}

canaddr( p ) NODE *p; {
	register int o = p->in.op;

	if( o==NAME || o==REG || o==ICON || o==OREG || (o==UNARY MUL && shumul(p->in.left)) ) return(1);
	return(0);
	}

flshape( p ) register NODE *p; {
	register o = p->in.op;
	if( o==NAME || o==REG || o==ICON || o==OREG ) return( 1 );
	return( o==UNARY MUL && shumul(p->in.left)==STARNM );
	}

shtemp( p ) register NODE *p; {
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == REG || p->in.op == OREG ) return( !istreg( p->tn.rval ) );
	return( p->in.op == NAME || p->in.op == ICON );
	}

spsz( t, v ) TWORD t; CONSZ v; {

	/* is v the size to increment something of type t */

	if( !ISPTR(t) ) return( 0 );
	t = DECREF(t);

	if( ISPTR(t) ) return( v == 2 );

	switch( t ){

	case UCHAR:
	case CHAR:
		return( v == 1 );

	case INT:
	case UNSIGNED:
		return( v == 2 );

	case FLOAT:
		return( v == 4 );

	case DOUBLE:
		return( v == 8 );
		}

	return( 0 );
	}

shumul( p ) register NODE *p; {
	register o;

	o = p->in.op;
	if( o == NAME || o == OREG || o == ICON ) return( STARNM );

	if( ( o == INCR || o == ASG MINUS ) &&
	    ( p->in.left->in.op == REG && p->in.right->in.op == ICON ) &&
	    p->in.right->in.name[0] == '\0' &&
	    spsz( p->in.left->in.type, p->in.right->tn.lval ) )
		return( STARREG );

	return( 0 );
	}

adrcon( val ) CONSZ val; {
	printf( CONFMT, val );
	}

conput( p ) register NODE *p; {
	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		putstr( rnames[p->tn.rval] );
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) NODE *p; {
	cerror( "insput" );
	}

upput( p ) NODE *p; {
	/* output the address of the second word in the
	   pair pointed to by p (for LONGs)*/
	CONSZ save;

	if( p->in.op == FLD ){
		p = p->in.left;
		}

	save = p->tn.lval;
	switch( p->in.op ){

	case NAME:
		p->tn.lval += SZINT/SZCHAR;
		acon( p );
		break;

	case ICON:
		/* addressable value of the constant */
		p->tn.lval &= BITMASK(SZINT);
		putstr( "$" );
		acon( p );
		break;

	case REG:
		putstr( rnames[p->tn.rval+1] );
		break;

	case OREG:
		p->tn.lval += SZINT/SZCHAR;
		if( p->tn.rval == R5 ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			}
		if( p->tn.lval != 0 || p->in.name[0] != '\0' ) acon( p );
		printf( "(%s)", rnames[p->tn.rval] );
		break;

	default:
		cerror( "illegal upper address" );
		break;

		}
	p->tn.lval = save;

	}

adrput( p ) register NODE *p; {
	/* output an address, with offsets, from p */

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		return;

	case ICON:
		/* addressable value of the constant */
		if( szty( p->in.type ) == 2 ) {
			/* print the high order value */
			CONSZ save;
			save = p->tn.lval;
			p->tn.lval = ( p->tn.lval >> SZINT ) & BITMASK(SZINT);
			putstr( "$" );
			acon( p );
			p->tn.lval = save;
			return;
			}
		printf( "$" );
		acon( p );
		return;

	case REG:
		putstr( rnames[p->tn.rval] );
		return;

	case OREG:
		if( p->tn.rval == R5 ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			printf( CONFMT, p->tn.lval );
			putstr( ".(r5)" );
			return;
			}
		if( p->tn.lval != 0 || p->in.name[0] != '\0' ) acon( p );
		printf( "(%s)", rnames[p->tn.rval] );
		return;

	case UNARY MUL:
		/* STARNM or STARREG found */
		if( tshape(p, STARNM) ) {
			putstr( "*" );
			adrput( p->in.left);
			}
		else {	/* STARREG - really auto inc or dec */
			/* turn into OREG so replacement node will
			   reflect the value of the expression */
			register i;
			register NODE *q, *l;

			l = p->in.left;
			q = l->in.left;
			p->in.op = OREG;
			p->in.rall = q->in.rall;
			p->tn.lval = q->tn.lval;
			p->tn.rval = q->tn.rval;
#ifndef FLEXNAMES
			for(i=0; i<NCHNAM; ++i)
				p->in.name[i] = q->in.name[i];
#else
			p->in.name = q->in.name;
#endif
			if( l->in.op == INCR ) {
				adrput( p );
				putstr( "+" );
				p->tn.lval -= l->in.right->tn.lval;
				}
			else {	/* l->in.op == ASG MINUS */
				putstr( "-" );
				adrput( p );
				}
			tfree( l );
		}
		return;

	default:
		cerror( "illegal address" );
		return;

		}

	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){	/* constant only */
		printf( CONFMT, p->tn.lval);
		printf( "." );
		}
	else if( p->tn.lval == 0 ) {	/* name only */
#ifndef FLEXNAMES
		printf( "%.8s", p->in.name );
#else
		putstr( p->in.name );
#endif
		}
	else {				/* name + offset */
#ifndef	FLEXNAMES
		printf( "%.8s+", p->in.name );
#else
		putstr( p->in.name );
#endif
		printf( CONFMT, p->tn.lval );
		printf( "." );
		}
	}

genscall( p, cookie ) register NODE *p; {
	/* structure valued call */
	return( gencall( p, cookie ) );
	}

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p */
	register temp;
	register m;

	if( p->in.right ) temp = argsize( p->in.right );
	else temp = 0;

	if( p->in.right ){ /* generate args */
		genargs( p->in.right );
		}

	if( !shltype( p->in.left->in.op, p->in.left ) ) {
		order( p->in.left, INAREG|SOREG );
		}

	p->in.op = UNARY CALL;
	m = match( p, INTAREG|INTBREG );
	popargs( temp );
	return(m != MDONE);
	}

popargs( size ) register size; {
	/* pop arguments from stack */

	toff -= size/2;
	if( toff == 0 && size >= 2 ) size -= 2;
	switch( size ) {
	case 0:
		break;
	case 2:
		printf( "	tst	(sp)+\n" );
		break;
	case 4:
		printf( "	cmp	(sp)+,(sp)+\n" );
		break;
	default:
		printf( "	add	$%d.,sp\n", size);
		}
	}

char *
ccbranches[] = {
	"	jeq	L%d\n",
	"	jne	L%d\n",
	"	jle	L%d\n",
	"	jlt	L%d\n",
	"	jge	L%d\n",
	"	jgt	L%d\n",
	"	jlos	L%d\n",
	"	jlo	L%d\n",
	"	jhis	L%d\n",
	"	jhi	L%d\n",
	};

/*	long branch table

   This table, when indexed by a logical operator,
   selects a set of three logical conditions required
   to generate long comparisons and branches.  A zero
   entry indicates that no branch is required.
   E.G.:  The <= operator would generate:
	cmp	AL,AR
	jlt	lable	/ 1st entry LT -> lable
	jgt	1f	/ 2nd entry GT -> 1f
	cmp	UL,UR
	jlos	lable	/ 3rd entry ULE -> lable
   1:
 */

int lbranches[][3] = {
	/*EQ*/	0,	NE,	EQ,
	/*NE*/	NE,	0,	NE,
	/*LE*/	LT,	GT,	ULE,
	/*LT*/	LT,	GT,	ULT,
	/*GE*/	GT,	LT,	UGE,
	/*GT*/	GT,	LT,	UGT,
	/*ULE*/	ULT,	UGT,	ULE,
	/*ULT*/	ULT,	UGT,	ULT,
	/*UGE*/	UGT,	ULT,	UGE,
	/*UGT*/	UGT,	ULT,	UGT,
	};

/* logical relations when compared in reverse order (cmp R,L) */
#ifndef	ONEPASS
short revrel[] ={ EQ, NE, GE, GT, LE, LT, UGE, UGT, ULE, ULT };
#else
extern short revrel[];
#endif

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */
	register *plb;
	int lab1f;

	if( o == 0 ) printf( "	jbr	L%d\n", lab );
	else	if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else {
		switch( brcase ) {

		case 'A':
		case 'C':
			plb = lbranches[ o-EQ ];
			lab1f = getlab();
			expand( brnode, FORCC, brcase=='C' ? "\tcmp\tAL,AR\n" : "\ttst\tAR\n" );
			if( *plb != 0 )
				printf( ccbranches[*plb-EQ], lab);
			if( *++plb != 0 )
				printf( ccbranches[*plb-EQ], lab1f);
			expand( brnode, FORCC, brcase=='C' ? "\tcmp\tUL,UR\n" : "\ttst\tUR\n" );
			printf( ccbranches[*++plb-EQ], lab);
			deflab( lab1f );
			reclaim( brnode, RNULL, 0 );
			break;

		default:
			if( mode=='F' ) o = revrel[ o-EQ ];
			printf( ccbranches[o-EQ], lab );
			break;
			}

		brcase = 0;
		brnode = 0;
		}
	}

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}

struct functbl {
	int fop;
	TWORD ftype;
	char *func;
	} opfunc[] = {
	MUL,		LONG,	"lmul",
	DIV,		LONG,	"ldiv",
	MOD,		LONG,	"lrem",
	ASG MUL,	LONG,	"almul",
	ASG DIV,	LONG,	"aldiv",
	ASG MOD,	LONG,	"alrem",
	MUL,		ULONG,	"lmul",
	DIV,		ULONG,	"uldiv",
	MOD,		ULONG,	"ulrem",
	ASG MUL,	ULONG,	"almul",
	ASG DIV,	ULONG,	"auldiv",
	ASG MOD,	ULONG,	"aulrem",
	0,	0,	0 };

hardops(p)  register NODE *p; {
	/* change hard to do operators into function calls.
	   for pdp11 do long * / %	*/
	register NODE *q;
	register struct functbl *f;
	register o;
	register TWORD t;

	o = p->in.op;
	t = p->in.type;
	if( t!=LONG && t!=ULONG ) return;

	for( f=opfunc; f->fop; f++ ) {
		if( o==f->fop && t==f->ftype ) goto convert;
		}
	return;

	/* need address of left node for ASG OP */
	/* WARNING - this won't work for long in a REG */
	convert:
	if( asgop( o ) ) {
		switch( p->in.left->in.op ) {

		case UNARY MUL:	/* convert to address */
			p->in.left->in.op = FREE;
			p->in.left = p->in.left->in.left;
			break;

		case NAME:	/* convert to ICON pointer */
			p->in.left->in.op = ICON;
			p->in.left->in.type = INCREF( p->in.left->in.type );
			break;

		case OREG:	/* convert OREG to address */
			p->in.left->in.op = REG;
			p->in.left->in.type = INCREF( p->in.left->in.type );
			if( p->in.left->tn.lval != 0 ) {
				q = talloc();
				q->in.op = PLUS;
				q->in.rall = NOPREF;
				q->in.type = p->in.left->in.type;
				q->in.left = p->in.left;
				q->in.right = talloc();

				q->in.right->in.op = ICON;
				q->in.right->in.rall = NOPREF;
				q->in.right->in.type = INT;
#ifdef	FLEXNAMES
				q->in.right->in.name = "";
#else
				q->in.right->in.name[0] = '\0';
#endif
				q->in.right->tn.lval = p->in.left->tn.lval;
				q->in.right->tn.rval = 0;

				p->in.left->tn.lval = 0;
				p->in.left = q;
				}
			break;

		default:
			cerror( "Bad address for hard ops" );
			/* NO RETURN */

			}
		}

	/* build comma op for args to function */
	q = talloc();
	q->in.op = CM;
	q->in.rall = NOPREF;
	q->in.type = INT;
	q->in.left = p->in.left;
	q->in.right = p->in.right;
	p->in.op = CALL;
	p->in.right = q;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
#ifndef	FLEXNAMES
	strcpy( q->in.name, f->func );
#else
	q->in.name = f->func;
#endif
	q->tn.lval = 0;
	q->tn.rval = 0;

	return;

	}

optim2( p ) register NODE *p; {
	/* do local tree transformations and optimizations */

	register NODE *r;

	switch( p->in.op ) {

	case AND:
		/* commute L and R to eliminate compliments and constants */
		if( p->in.left->in.op==ICON || p->in.left->in.op==COMPL ) {
			r = p->in.left;
			p->in.left = p->in.right;
			p->in.right = r;
			}
	case ASG AND:
		/* change meaning of AND to ~R&L - bic on pdp11 */
		r = p->in.right;
		if( r->in.op==ICON ) { /* compliment constant */
			r->tn.lval = ~r->tn.lval;
			}
		else if( r->in.op==COMPL ) { /* ~~A => A */
			r->in.op = FREE;
			p->in.right = r->in.left;
			}
		else { /* insert complement node */
			p->in.right = talloc();
			p->in.right->in.op = COMPL;
			p->in.right->in.rall = NOPREF;
			p->in.right->in.type = r->in.type;
			p->in.right->in.left = r;
			p->in.right->in.right = NULL;
			}
		break;

		}
	}

special( p, shape ) register NODE *p; {
	/* special shape matching routine */

	switch( shape ) {

	case SSCON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= -128 && p->tn.lval <=127 ) return( 1 );
		break;

	case SCCON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= 0 && p->tn.lval <=32767 ) return( 1 );
		break;

	default:
		cerror( "bad special shape" );

		}

	return( 0 );
	}

NODE * addroreg(l) NODE *l;
				/* OREG was built in clocal()
				 * for an auto or formal parameter
				 * now its address is being taken
				 * local code must unwind it
				 * back to PLUS/MINUS REG ICON
				 * according to local conventions
				 */
{
	cerror("address of OREG taken");
	/*NOTREACHED*/
}

# ifndef ONEPASS
main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}
# endif

myreader(p) register NODE *p; {
	walkf( p, hardops );	/* convert ops to function calls */
	canon( p );		/* expands r-vals for fileds */
	walkf( p, optim2 );
	toff = 0;  /* stack offset swindle */
	}

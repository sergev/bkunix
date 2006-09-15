# include "pass1.h"
# include <sys/types.h>
# include <a.out.h>
# include <stab.h>

int proflg = 0;	/* are we generating profiling code? */
int strftn = 0;  /* is the current function one which returns a value */
int gdebug;
int fdefflag;  /* are we within a function definition ? */
char NULLNAME[8];
int labelno;

# define putstr(s)	fputs((s), stdout)

branch( n ){
	/* output a branch to label n */
	/* exception is an ordinary function branching to retlab: then, return */
	if( n == retlab && !strftn ){
		putstr( "	jmp	cret\n" );
		}
	else printf( "	jbr	L%d\n", n );
	}

int lastloc = { -1 };

defalign(n) {
	/* cause the alignment to become a multiple of n */
	n /= SZCHAR;
	if( lastloc != PROG && n > 1 ) printf( "	.even\n" );
	}

locctr( l ){
	register temp;
	/* l is PROG, ADATA, DATA, STRNG, ISTRNG, or STAB */

	if( l == lastloc ) return(l);
	temp = lastloc;
	lastloc = l;
	switch( l ){

	case PROG:
		putstr( "	.text\n" );
		psline();
		break;

	case DATA:
	case ADATA:
		putstr( "	.data\n" );
		break;

	case STRNG:
	case ISTRNG:
		break;

	case STAB:
		cerror( "locctr: STAB unused" );
		break;

	default:
		cerror( "illegal location counter" );
		}

	return( temp );
	}

deflab( n ){
	/* output something to define the current position as label n */
	printf( "L%d:\n", n );
	}

int crslab = 10;

getlab(){
	/* return a number usable for a label */
	return( ++crslab );
	}

efcode(){
	/* code for the end of a function */

	if( strftn ){  /* copy output (in R0) to caller */
		register NODE *l, *r;
		register struct symtab *p;
		register TWORD t;
		int i;

		p = &stab[curftn];
		t = p->stype;
		t = DECREF(t);

		deflab( retlab );

		i = getlab();	/* label for return area */
#ifndef LCOMM
		putstr("	.bss\n" );
		printf("L%d: .=.+%d.\n", i, tsize(t, p->dimoff, p->sizoff)/SZCHAR );
		putstr("	.text\n" );
#else
		{ int sz = tsize(t, p->dimoff, p->sizoff) / SZCHAR;
		if (sz % sizeof (int))
			sz += sizeof (int) - (sz % sizeof (int));
		printf("	.lcomm	L%d,%d\n", i, sz);
		}
#endif
		psline();
		printf("	mov	$L%d,r1\n", i);

		reached = 1;
		l = block( REG, NIL, NIL, PTR|t, p->dimoff, p->sizoff );
		l->tn.rval = 1;  /* R1 */
		l->tn.lval = 0;  /* no offset */
		r = block( REG, NIL, NIL, PTR|t, p->dimoff, p->sizoff );
		r->tn.rval = 0;  /* R0 */
		r->tn.lval = 0;
		l = buildtree( UNARY MUL, l, NIL );
		r = buildtree( UNARY MUL, r, NIL );
		l = buildtree( ASSIGN, l, r );
		l->in.op = FREE;
		ecomp( l->in.left );
		printf( "	mov	$L%d,r0\n", i );
		/* turn off strftn flag, so return sequence will be generated */
		strftn = 0;
		}
	branch( retlab );
	p2bend();
	fdefflag = 0;
	}

bfcode( a, n ) int a[]; {
	/* code for the beginning of a function; a is an array of
		indices in stab for the arguments; n is the number */
	register i;
	register temp;
	register struct symtab *p;
	OFFSZ off;

	locctr( PROG );
	p = &stab[curftn];
	defnam( p );
	temp = p->stype;
	temp = DECREF(temp);
	strftn = (temp==STRTY) || (temp==UNIONTY);

	retlab = getlab();

	/* routine prolog */

	if( proflg ){	/* profile code */
		i = getlab();
		printf( "	mov	$L%d,r0\n", i );
		printf( "	jsr	pc,mcount\n" );
		printf( "	.bss\nL%d:	.=.+2\n	.text\n", i );
		}

	printf( "	jsr	r5,csv\n" );
	/* adjust stack for autos */
	printf( "	sub	$.F%d,sp\n", ftnno );

	off = ARGINIT;

	for( i=0; i<n; ++i ){
		p = &stab[a[i]];
		if( p->sclass == REGISTER ){
			temp = p->offset;  /* save register number */
			p->sclass = PARAM;  /* forget that it is a register */
			p->offset = NOOFFSET;
			oalloc( p, &off );
		printf( "	mov	%d.(r5),r%d\n", p->offset/SZCHAR, temp);
			p->offset = temp;  /* remember register number */
			p->sclass = REGISTER;   /* remember that it is a register */
			}
		else if( p->stype == STRTY || p->stype == UNIONTY ) {
			p->offset = NOOFFSET;
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			SETOFF( off, ALSTACK );
			}
		else {
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			}

		}
	if (gdebug) {
#ifdef STABDOT
		pstabdot(N_SLINE, lineno);
#else
		pstab(NULLNAME, N_SLINE);
		printf("0,%d,LL%d\n", lineno, labelno);
		printf("LL%d:\n", labelno++);
#endif
	}
	fdefflag = 1;
	}

bccode(){ /* called just before the first executable statment */
		/* by now, the automatics and register variables are allocated */
	SETOFF( autooff, SZINT );
	/* set aside store area offset */
	p2bbeg( autooff, regvar );
	}

ejobcode( flag ){
	/* called just before final exit */
	/* flag is 1 if errors, 0 if none */
	}

aobeg(){
	/* called before removing automatics from stab */
	}

aocode(p) struct symtab *p; {
	/* called when automatic p removed from stab */
	}

aoend(){
	/* called after removing all automatics from stab */
	}

defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */

	if( p->sclass == EXTDEF ){
		printf( "	.globl	%s\n", exname( p->sname ) );
		}
	if( p->sclass == STATIC && p->slevel>1 ) deflab( (int)p->offset );
	else printf( "%s:\n", exname( p->sname ) );

	}

bycode( t, i ){
#ifdef ASSTRINGS
static	int	lastoctal = 0;
#endif

	/* put byte i+1 in a string */

#ifdef ASSTRINGS

	i &= 077;
	if ( t < 0 ){
		if ( i != 0 )	putstr( "\"\n" );
	} else {
		if ( i == 0 ) putstr("\t.ascii\t\"");
		if ( t == '\\' || t == '"'){
			lastoctal = 0;
			printf("\\%c", t);
		}
			/*
			 *	We escape the colon in strings so that
			 *	c2 will, in its infinite wisdom, interpret
			 *	the characters preceding the colon as a label.
			 *	If we didn't escape the colon, c2 would
			 *	throw away any trailing blanks or tabs after
			 *	the colon, but reconstruct a assembly
			 *	language semantically correct program.
			 *	C2 hasn't been taught about strings.
			 */
		else if ( t == ':' || t < 040 || t >= 0177 ){
			lastoctal++;
			printf("\\%o",t);
		}
		else if ( lastoctal && '0' <= t && t <= '9' ){
			lastoctal = 0;
			printf("\"\n\t.ascii\t\"%c", t );
		}
		else
		{	
			lastoctal = 0;
			putchar(t);
		}
		if ( i == 077 ) putstr("\"\n");
	}
#else

	i &= 07;
	if( t < 0 ){ /* end of the string */
		if( i != 0 ) putchar( '\n' );
		}

	else { /* stash byte t into string */
		if( i == 0 ) putstr( "	.byte	" );
		else putchar( ',' );
		printf( "0x%x", t );
		if( i == 07 ) putchar( '\n' );
		}
#endif
	}

zecode( n ){
	/* n integer words of zeros */
	OFFSZ temp;
	register i;

	if( n <= 0 ) return;
	printf( "	" );
	for( i=1; i<n; i++ ) {
		if( i%8 == 0 )
			printf( "\n	" );
		printf( "0; " );
		}
	printf( "0\n" );
	temp = n;
	inoff += temp*SZINT;
	}

fldal( t ) unsigned t; { /* return the alignment of field of type t */
	uerror( "illegal field type" );
	return( ALINT );
	}

fldty( p ) struct symtab *p; { /* fix up type of field p */
	;
	}

where(c){ /* print location of error  */
	/* c is either 'u', 'c', or 'w' */
	/* GCOS version */
	fprintf( stderr, "%s, line %d: ", ftitle, lineno );
	}

main( argc, argv ) char *argv[]; {
#ifdef BUFSTDERR
	char errbuf[BUFSIZ];
	setbuf(stderr, errbuf);
#endif
	return(mainp1( argc, argv ));
	}

struct sw heapsw[SWITSZ];	/* heap for switches */

genswitch(p,n) register struct sw *p;{
	/*	p points to an array of structures, each consisting
		of a constant value and a label.
		The first is >=0 if there is a default label;
		its value is the label number
		The entries p[1] to p[n] are the nontrivial cases
		*/
	register i;
	register CONSZ j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	if( range>0 && range <= 3*n && n>=4 ){ /* implement a direct switch */

		swlab = getlab();
		dlab = p->slab >= 0 ? p->slab : getlab();

		if( p[1].sval ){
			printf( "	sub	$" );
			printf( CONFMT, p[1].sval );
			printf( ".,r0\n" );
			}

		/* note that this is a cl; it thus checks
		   for numbers below range as well as out of range.
		   */
		printf( "	cmp	r0,$%ld.\n", range );
		printf( "	jhi	L%d\n", dlab );

		printf( "	asl	r0\n" );
		printf( "	jmp	*L%d(r0)\n", swlab );

		/* output table */

		locctr( ADATA );
		defalign( ALPOINT );
		deflab( swlab );

		for( i=1,j=p[1].sval; i<=n; ++j ){

			printf( "	L%d\n", ( j == p[i].sval ) ?
				p[i++].slab : dlab );
			}

		locctr( PROG );

		if( p->slab< 0 ) deflab( dlab );
		return;

		}

	if( n>8 ) {	/* heap switch */

		heapsw[0].slab = dlab = p->slab >= 0 ? p->slab : getlab();
		makeheap(p, n, 1);	/* build heap */

		walkheap(1, n);	/* produce code */

		if( p->slab >= 0 )
			branch( dlab );
		else
			printf("L%d:\n", dlab);
		return;
	}

	/* debugging code */

	/* out for the moment
	if( n >= 4 ) werror( "inefficient switch: %d, %d", n, (int) (range/n) );
	*/

	/* simple switch code */

	for( i=1; i<=n; ++i ){
		/* already in r0 */

		putstr( "	cmp	r0,$" );
		printf( CONFMT, p[i].sval );
		printf( ".\n	jeq	L%d\n", p[i].slab );
		}

	if( p->slab>=0 ) branch( p->slab );
	}

makeheap(p, m, n)
register struct sw *p;
{
	register int q;

	q = select(m);
	heapsw[n] = p[q];
	if( q>1 ) makeheap(p, q-1, 2*n);
	if( q<m ) makeheap(p+q, m-q, 2*n+1);
}

select(m) {
	register int l,i,k;

	for(i=1; ; i*=2)
		if( (i-1) > m ) break;
	l = ((k = i/2 - 1) + 1)/2;
	return( l + (m-k < l ? m-k : l));
}

walkheap(start, limit)
{
	int label;


	if( start > limit ) return;
	printf("	cmp	r0,$%d.\n",  heapsw[start].sval);
	printf("	jeq	L%d\n", heapsw[start].slab);
	if( (2*start) > limit ) {
		printf("	jbr 	L%d\n", heapsw[0].slab);
		return;
	}
	if( (2*start+1) <= limit ) {
		label = getlab();
		printf("	jgt	L%d\n", label);
	} else
		printf("	jgt	L%d\n", heapsw[0].slab);
	walkheap( 2*start, limit);
	if( (2*start+1) <= limit ) {
		printf("L%d:\n", label);
		walkheap( 2*start+1, limit);
	}
}

# include "pass2.h"

int fltused = 0;

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	return( shltype(p->in.left->in.op, p->in.left ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
	if( p->in.op == INCR && p->in.left->in.op == REG && spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		/* STARREG */
		return( 0 );
		}

	p = p->in.left;
	if( p->in.op == UNARY MUL ) p = p->in.left;
	return( p->in.op == NAME || p->in.op == OREG || p->in.op == REG );
	}

autoincr( p ) NODE *p; {
	register NODE *q = p->in.left;

	if( q->in.op == INCR && q->in.left->in.op == REG &&
	    ISPTR(q->in.type) && p->in.type == DECREF(q->in.type) &&
	    tlen(p) == q->in.right->tn.lval ) return(1);

	return(0);
	}

mkadrs(p) register NODE *p; {
	register o;

	o = p->in.op;

	if( asgop(o) ){
		if( p->in.left->in.su >= p->in.right->in.su ){
			if( p->in.left->in.op == UNARY MUL ){
				if( p->in.left->in.su > 0 )
					SETSTO( p->in.left->in.left, INTEMP );
				else {
					if( p->in.right->in.su > 0 ) SETSTO( p->in.right, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->in.left, INTEMP );
				}
			}
		else SETSTO( p->in.right, INTEMP );
		}
	else {
		if( p->in.left->in.su > p->in.right->in.su ){
			SETSTO( p->in.left, INTEMP );
			}
		else {
			SETSTO( p->in.right, INTEMP );
			}
		}
	}

notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	/* offset of off, (from a register of r), if the
	/* resulting thing had type t */

	/* if( r == R0 ) return( 1 );  /* NO */
	return(0);  /* YES */
	}

# define max(x,y) ((x)<(y)?(y):(x))
# define min(x,y) ((x)<(y)?(x):(y))


# define ZCHAR 01
# define ZLONG 02
# define ZFLOAT 04

zum( p, zap ) register NODE *p; {
	/* zap Sethi-Ullman number for chars, longs, floats */
	/* in the case of longs, only STARNM's are zapped */
	/* ZCHAR, ZLONG, ZFLOAT are used to select the zapping */

	register su;

	su = p->in.su;

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su == 0 ) p->in.su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		if( p->in.op == UNARY MUL && su == 0 ) p->in.su = su = 2;
		break;

	case FLOAT:
		if( !(zap&ZFLOAT) ) break;
		if( su == 0 ) p->in.su = su = 1;

		}

	return( su );
	}

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	   number, or local equivalent */

	register o, ty, sul, sur;
	register nr;

	ty = optype( o=p->in.op);
	nr = szty( p->in.type );
	p->in.su = 0;

	if( ty == LTYPE ) {
		if( p->in.type==FLOAT ) p->in.su = 1;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->in.su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->in.left ) ) return;

		default:
			p->in.su = max( p->in.left->in.su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		if( sul==0 ){
			/* don't need to worry about the left side */
			p->in.su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				/* just get the lhs address into a register, and mov */
				/* the `nr' covers the case where value is in reg afterwards */
				p->in.su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->in.su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->in.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->in.su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		/* left then right, max(sul,sur+nr) */
		/* right then left, max(sur,sul+nr) */
		/* to hold both sides in regs: nr+nr */
		nr = szty( p->in.left->in.type );
		sul = zum( p->in.left, ZLONG|ZCHAR|ZFLOAT );
		sur = zum( p->in.right, ZLONG|ZCHAR|ZFLOAT );
		p->in.su = min( max(sul,sur+nr), max(sur,sul+nr) );
		return;
		}

	if( asgop(o) ){
		/* computed by doing right, doing left address, doing left, op, and store */
		switch( o ) {
		case INCR:
		case DECR:
			/* do as binary op */
			break;

		case ASG DIV:
		case ASG MOD:
		case ASG MUL:
			if( p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
			goto gencase;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:  /* really bic */
		case ASG OR:
			if( p->in.type == INT || p->in.type == UNSIGNED || ISPTR(p->in.type) ) goto asop;

		gencase:
		default:
			sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
			if( sur == 0 ){ /* easy case: if addressable,
				do left value, op, store */
				if( sul == 0 ) p->in.su = nr;
				/* harder: left adr, val, op, store */
				else p->in.su = max( sul, nr+1 );
				}
			else { /* do right, left adr, left value, op, store */
				if( sul == 0 ){  /* right, left value, op, store */
					p->in.su = max( sur, nr+nr );
					}
				else {
					p->in.su = max( sur, max( sul+nr, 1+nr+nr ) );
					}
				}
			return;
			}
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->in.su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MOD || o==MUL )
	    && p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
	if( o==PLUS || o==MUL || o==OR || o==ER ){
		/* AND is ruined by the hardware */
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->in.left ) || sul > sur ) goto noswap;  /* don't do it! */

		/* look for a funny type on the left, one on the right */


		lt = p->in.left->in.type;
		rt = p->in.right->in.type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR) && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->in.left->in.op == UNARY MUL && sul==0 ) goto noswap;
				if( p->in.right->in.op == UNARY MUL && p->in.left->in.op != UNARY MUL ) goto swap;
				goto noswap;
				}
			else if( p->in.left->in.op == UNARY MUL && sul == 0 ) goto noswap;
			else goto swap;  /* put long on right, unless STARNM */
			}

		/* we are finished with the type stuff now; if one is addressable,
			put it on the right */
		if( sul == 0 && sur != 0 ){

			NODE *s;
			int ssu;

		swap:
			ssu = sul;  sul = sur; sur = ssu;
			s = p->in.left;  p->in.left = p->in.right; p->in.right = s;
			}
		}
	noswap:

	sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
	if( sur == 0 ){
		/* get left value into a register, do op */
		p->in.su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->in.su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->in.op == FLD ){
		p->in.left->in.rall = p->in.rall;
		p = p->in.left;
		}

	if( p->in.op != UNARY MUL ) return;  /* no more to do */
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		p->in.rall = r;
		p = p->in.left;
		}
	if( p->in.op == PLUS && p->in.right->in.op == ICON ){
		p->in.rall = r;
		p = p->in.left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register o, type, down1, down2, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;
	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( o = p->in.op );
	type = p->in.type;


	if( type == DOUBLE || type == FLOAT ){
		if( o == FORCE ) down1 = FR0|MUSTDO;
		++fltused;
		}
	else switch( o ) {
	case ASSIGN:	
		down1 = NOPREF;
		down2 = down;
		break;

	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
		/* keep the addresses out of the hair of (r0,r1) */
		if(fregs == 2 ){
			/* lhs in (r0,r1), nothing else matters */
			down1 = R1|MUSTDO;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in (r0,r1), address of left in r2 */
		p->in.left->in.rall = R1|MUSTDO;
		mkrall( p->in.left, R2|MUSTDO );
		/* now, deal with right */
		if( fregs == 3 ) rallo( p->in.right, NOPREF );
		else {
			/* put address of long or value here */
			p->in.right->in.rall = R3|MUSTDO;
			mkrall( p->in.right, R3|MUSTDO );
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		rallo( p->in.left, R1|MUSTDO );

		if( fregs == 2 ){
			rallo( p->in.right, NOPREF );
			return;
			}
		/* compute addresses, stay away from (r0,r1) */

		p->in.right->in.rall = (fregs==3) ? R2|MUSTDO : R3|MUSTDO ;
		mkrall( p->in.right, R2|MUSTDO );
		return;

	case CALL:
	case STASG:
	case EQ:
	case NE:
	case GT:
	case GE:
	case LT:
	case LE:
	case NOT:
	case ANDAND:
	case OROR:
		down1 = NOPREF;
		break;

	case FORCE:	
		down1 = R0|MUSTDO;
		break;

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

offstar( p ) register NODE *p; {
	if( p->in.op == PLUS ) {
		if( p->in.left->in.su == fregs ) {
			order( p->in.left, INTAREG|INAREG );
			return;
		} else if( p->in.right->in.su == fregs ) {
			order( p->in.right, INTAREG|INAREG );
			return;
		}
		if( p->in.left->in.op==LS && 
		  (p->in.left->in.left->in.op!=REG || tlen(p->in.left->in.left)!=sizeof(int) ) ) {
			order( p->in.left->in.left, INTAREG|INAREG );
			return;
		}
		if( p->in.right->in.op==LS &&
		  (p->in.right->in.left->in.op!=REG || tlen(p->in.right->in.left)!=sizeof(int) ) ) {
			order( p->in.right->in.left, INTAREG|INAREG );
			return;
		}
		if( p->in.type == (PTR|CHAR) || p->in.type == (PTR|UCHAR) ) {
			if( p->in.left->in.op!=REG || tlen(p->in.left)!=sizeof(int) ) {
				order( p->in.left, INTAREG|INAREG );
				return;
			}
			else if( p->in.right->in.op!=REG || tlen(p->in.right)!=sizeof(int) ) {
				order(p->in.right, INTAREG|INAREG);
				return;
			}
		}
	}
	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON ){
			p = p->in.left;
			order( p , INTAREG|INAREG);
			return;
			}
		}

	if( p->in.op == UNARY MUL && !canaddr(p) ) {
		offstar( p->in.left );
		return;
	}

	order( p, INTAREG|INAREG );
	}

setincr( p ) register NODE *p; {
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		offstar( p );
		return( 1 );
		}
	return( 0 );
	}

niceuty( p ) register NODE *p; {
	register TWORD t;

	return( p->in.op == UNARY MUL && (t=p->in.type)!=CHAR &&
		t!= UCHAR && t!= FLOAT &&
		shumul( p->in.left) != STARREG );
	}
setbin( p ) register NODE *p; {
	register NODE *r, *l;

	r = p->in.right;
	l = p->in.left;

	if( p->in.right->in.su == 0 ){ /* rhs is addressable */
		if( logop( p->in.op ) ){
			if( l->in.op == UNARY MUL && l->in.type != FLOAT && shumul( l->in.left ) != STARREG ) offstar( l->in.left );
			else order( l, INAREG|INTAREG|INBREG|INTBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->in.op ) ){
		/* relational: do both sides into regs if need be */

		if( r->in.su > l->in.su ){
			if( niceuty(r) ){
				offstar( r->in.left );
				return( 1 );
				}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
				return( 1 );
				}
			}
		if( niceuty(l) ){
			offstar( l->in.left );
			return( 1 );
			}
		else if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );
			}
		else if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		cerror( "setbin can't deal with %s", opst[p->in.op] );
		}

	/* ordinary operator */

	if( !istnode(r) && r->in.su > l->in.su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );  /* hopefully, it is addressable by now */
			}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
		}
	else {
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	}

setstr( p ) register NODE *p; { /* structure assignment */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INTAREG );
		return(1);
		}
	p = p->in.left;
	if( p->in.op != NAME && p->in.op != OREG ){
		if( p->in.op != UNARY MUL ) cerror( "bad setstr" );
		order( p->in.left, INTAREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* setup for assignment operator */

	if( p->in.right->in.su != 0 && p->in.right->in.op != REG ) {
		if( p->in.right->in.op == UNARY MUL )
			offstar( p->in.right->in.left );
		else
			order( p->in.right, INAREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	if( p->in.right->in.op != REG && ( p->in.type == FLOAT || p->in.type == DOUBLE ) ) {
		order( p->in.right, INBREG );
		return(1);
		}
	if( p->in.left->in.op == UNARY MUL && !tshape( p->in.left, STARREG|STARNM ) ){
		offstar( p->in.left->in.left );
		return(1);
		}
	if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
		offstar( p->in.left->in.left->in.left );
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INAREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* setup for =ops */
	register sul, sur;
	register NODE *q, *p2;

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	switch( p->in.op ){

	case ASG PLUS:
	case ASG OR:
	case ASG MINUS:
		if( p->in.type != INT && p->in.type != UNSIGNED && !ISPTR(p->in.type) ) break;
		if( p->in.right->in.type == CHAR || p->in.right->in.type == UCHAR ){
			order( p->in.right, INAREG );
			return( 1 );
			}
		break;

	case ASG ER:
		if( sul == 0 || p->in.left->in.op == REG ){
			if( p->in.left->in.type == CHAR || p->in.left->in.type == UCHAR ) goto rew;  /* rewrite */
			order( p->in.right, INAREG|INBREG );
			return( 1 );
			}
		goto leftadr;
		}

	if( sur == 0 ){

	leftadr:
		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) goto rew;  /* rewrite */

		/* harder; make aleft address, val, op, and store */

		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left );
			return( 1 );
			}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left->in.left );
			return( 1 );
			}
	rew:	/* rewrite, accounting for autoincrement and autodecrement */

		q = p->in.left;
		if( q->in.op == FLD ) q = q->in.left;
		if( q->in.op != UNARY MUL || shumul(q->in.left) != STARREG ) return(0); /* let reader.c do it */

		/* mimic code from reader.c */

		p2 = tcopy( p );
		p->in.op = ASSIGN;
		reclaim( p->in.right, RNULL, 0 );
		p->in.right = p2;

		/* now, zap INCR on right, ASG MINUS on left */

		if( q->in.left->in.op == INCR ){
			q = p2->in.left;
			if( q->in.op == FLD ) q = q->in.left;
			if( q->in.left->in.op != INCR ) cerror( "bad incr rewrite" );
			}
		else if( q->in.left->in.op != ASG MINUS )  cerror( " bad -= rewrite" );

		q->in.left->in.right->in.op = FREE;
		q->in.left->in.op = FREE;
		q->in.left = q->in.left->in.left;

		/* now, resume reader.c rewriting code */

		canon(p);
		rallo( p, p->in.rall );
		order( p2->in.left, INTBREG|INTAREG );
		order( p2, INTBREG|INTAREG );
		return( 1 );
		}

	/* harder case: do right, left address, left value, op, store */

	if( p->in.right->in.op == UNARY MUL ){
		offstar( p->in.right->in.left );
		return( 1 );
		}
	/* sur> 0, since otherwise, done above */
	if( p->in.right->in.op == REG ) goto leftadr;  /* make lhs addressable */
	order( p->in.right, INAREG|INBREG );
	return( 1 );
	}

int crslab = 32767;

getlab(){
	return( crslab-- );
	}

deflab( l ){
	printf( "L%d:\n", l );
	}

genargs( p ) register NODE *p; {
	register NODE *pasg;
	register align;
	register size;
	int count;

	/* generate code for the arguments */

	/*  first, do the arguments on the right */
	while( p->in.op == CM ){
		genargs( p->in.right );
		p->in.op = FREE;
		p = p->in.left;
		}

	if( p->in.op == STARG ){ /* structure valued argument */

		size = p->stn.stsize;
		align = p->stn.stalign;
		if( p->in.left->in.op == ICON ){
			p->in.op = FREE;
			p = p->in.left;
			}
		else {
			/* make it look beautiful... */
			p->in.op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			for( count = 0; p->in.op != OREG && count < 10; ++count ){
				offstar( p->in.left );
				canon( p );
				}
			if( p->in.op != OREG ) cerror( "stuck starg" );
			}

		pasg = talloc();
		pasg->in.op = STARG;
		pasg->in.rall = NOPREF;
		pasg->stn.stsize = size;
		pasg->stn.stalign = align;
		pasg->in.left = p;

 		order( pasg, FORARG );
		return;
		}

	/* ordinary case */

	order( p, FORARG );
	}

OFFSZ
argsize( p ) register NODE *p; {
	OFFSZ t;
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
		}
	if( p->in.type == DOUBLE || p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if( p->in.type == LONG || p->in.type == ULONG ) {
		SETOFF( t, 2);
		return( t+4 );
		}
	else if( p->in.op == STARG ){
		SETOFF( t, p->stn.stalign );  /* alignment */
		return( t + p->stn.stsize );  /* size */
		}
	else {
		SETOFF( t, 2 );
		return( t+2 );
		}
	}

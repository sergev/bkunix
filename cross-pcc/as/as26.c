/*
	AS - PDP/11 Assembler, Part II

	opline - statment processing
*/

#include <stdio.h>
#include "as.h"
#include "as2.h"

void opline()
{
	struct value v;
	unsigned topcode, ttype, tb, a1;
	char *pf;
	struct value *ttok;

	/*
		Handle non symbol tokens
	*/
	if(tok.u < TOKSYMBOL) {
		if(tok.u == TOKFILE) {
			line = 1;
			for(pf = argb; agetw() && tok.u != 0xffff; ++pf) {
				*pf = tok.u;
			}
			*pf = '\0';
			return;
		}
		if(tok.u != '<') {
			v = express();
			outw(v.type.i,v.val.i);
			return;
		}
		goto opl17;
	}

	/*
		Handle non-opcode symbols
	*/
	tb = tok.v->type.b;
	if(tb == TYPEREGIS || tb == TYPEOPEST ||
	   tb == TYPEOPESD || tb <  TYPEOPFD ||
	   tb >  TYPEOPJCC) {
		v = express();
		outw(v.type.i,v.val.i);
		return;
	}

	/*
		Handle op codes
	*/
	topcode = tok.v->val.u;
	ttype = tb;
	readop();
	padrb = &adrbuf[0];
	swapf = 0;
	rlimit = -1;

	switch(ttype) {

	case TYPEOPFD:
		rlimit = 0400;
		op2a(topcode);
		return;

	case TYPEOPBR:
		v = express();

	dobranch:

		if(passno != 0) {
			v.val.i -= dot;
			if(v.val.i < -254 || v.val.i > 256 ||
			   (v.val.u & 1) || v.type.u != dotrel) {
			   if(DEBUG)
			      printf(" branch error val %d type %d dotrel %d ",
			         v.val.i,v.type.u,dotrel);
			   aerror('b');
			   v.val.u = 0;
			  }
			  else {
			  	v.val.u = ((v.val.u >> 1) -1) & 0377;
			  }
		}
		v.val.u |= topcode;
		outw(0,v.val.u);
		return;

	case TYPEOPJSR:
		v = express();
		checkreg(&v);
		readop();
		op2b(v.val.u,topcode);
		return;

	case TYPEOPRTS:
		v = express();
		checkreg(&v);
		v.val.u |= topcode;
		outw(v.type.u,v.val.u);
		return;

	case TYPEOPSYS:
		v = express();
		if(v.val.u >= 64 || v.type.u > TYPEABS)
			aerror('a');
		v.val.u |= topcode;
		outw(v.type.u,v.val.u);
		return;

	case TYPEOPMOVF:
		rlimit = 0400;
		a1 = address();
		if(a1 >= 4) {
			++swapf;
			readop();
			op2b(a1,topcode);
			return;
		}
		readop();
		op2b(a1,0174000);
		return;

	case TYPEOPDO:
		op2a(topcode);
		return;

	case TYPEOPFF:
		++swapf;
		rlimit = 0400;
		op2a(topcode);
		return;

	case TYPEOPSO:
		op2b(0,topcode);
		return;

	case TYPEOPBYTE:
		do {
			v = express();
			outb(v.type.u,v.val.u);
		} while(tok.u == ',' && (readop(),1));
		return;

	case TYPEOPWORD:
		do {
			v = express();
			outw(v.type.u,v.val.u);
		} while(tok.u == ',' && (readop(),1));
		return;

	case TYPEOPASC:
	opl17:
		agetw();
		while(tok.i >= 0) {
			outb(TYPEABS,tok.u & 0377);
			agetw();
		}
		agetw();
		return;

	case TYPEOPEVEN:
		if((dot & 1) == 0)
			return;
		if(dotrel == TYPEBSS) {
			++dot;
			return;
		}
		outb(0,0);
		return;


	case TYPEOPIF:
		express();
		return;

	case TYPEOPEIF:
		return;

	case TYPEOPGLB:
		while(tok.u >= TOKSYMBOL) {
			tok.v->type.b |= TYPEEXT;
			readop();
			if(tok.u != ',')
				break;
			readop();
		}
		return;

	case TYPEREGIS:
	case TYPEOPEST:
	case TYPEOPESD:
		v = express();
		outw(v.type.u,v.val.u);
		return;


	case TYPEOPTXT:
	case TYPEOPDAT:
	case TYPEOPBSS:
		dot = (dot + 1) & ~1;
		savdot[dotrel-TYPETXT] = dot;
		if(passno != 0) {
			flush(&txtp);
			flush(&relp);
			tseekp = &aseek[ttype-TYPEOPTXT];
			oset(&txtp, *tseekp);
			rseekp = &relseek[ttype-TYPEOPTXT];
			oset(&relp, *rseekp);
		}
		dot = savdot[ttype-TYPEOPTXT];
		dotrel = ttype-TYPEOPTXT+TYPETXT;
		return;

	case TYPEOPMUL:
		++swapf;
		rlimit = 01000;
		op2a(topcode);
		return;

	case TYPEOPSOB:
		v = express();
		checkreg(&v);
		tb = v.val.u;
		tb = (tb << 8) | ((tb >> 8) & 0xff);
		topcode |= (tb >> 2);
		readop();
		v = express();
		if(passno != 0) {
			v.val.u -= dot;
			v.val.i = -v.val.i;
			if(v.val.i < -2 || v.val.i > 0175) {
				aerror('b');
				outw(0,topcode);
				return;
			}
			v.val.u += 4;
			if((v.val.u & 1) || v.type.u != dotrel) {
				aerror('b');
				outw(0,topcode);
				return;
			}
			v.val.u = ((v.val.u >> 1) - 1) & 0377;
		}
		v.val.u |= topcode;
		outw(0,v.val.u);
		return;

	case TYPEOPCOM:
		if(tok.u >= TOKSYMBOL) {
			ttok = tok.v;
			readop();
			readop();
			v = express();
			if((ttok->type.u & 037) == TYPEUNDEF) {
				ttok->type.u |= TYPEEXT;
				ttok->val.u = v.val.u;
			}
		}
		return;

	case TYPEOPJBR:
	case TYPEOPJCC:
		v = express();
		if(passno == 0) {
			v.val.u = setbr(v.val.u);
			if(v.val.u != 0 && topcode != OPCODBR)
				v.val.u += 2;
			dot += v.val.u + 2;
			return;
		}
		if(getbr() == 0)
			goto dobranch;
		if(topcode != OPCODBR)
			outw(TYPEABS,topcode ^ 0402);
		outw(TYPEABS,OPCODJMP + 037);
		outw(v.type.u,v.val.u);
		return;

	default:
		v = express();
		outw(v.type.u,v.val.u);
		return;

	}
}


/*
	Routine to do a 2 operand op code
*/
void op2a(op)
	unsigned op;
{
	unsigned a1;

	a1 = address();
	readop();
	op2b(a1,op);
}


/*
	routine to do second (or only) operand
*/
void op2b(a1, op)
	unsigned a1, op;
{
	unsigned a2,t;
	unsigned *p;

	a2 = address();
	if(swapf) {
		t = a1;
		a1 = a2;
		a2 = t;
	}
	a1 = (a1 << 8) | ((a1 >> 8) & 0xff);
	a1 >>= 2;
	if(a1 >= rlimit)
		aerror('x');
	a2 |= a1 | op;
	outw(0,a2);
	for(p = &adrbuf[0]; p < padrb; p += 3) {
		xsymbol = p[2];
		outw(p[1],p[0]);
	}
	return;
}


/*
	Routine to process an address operand
*/
unsigned address()
{
	struct value v;
	int t;

	t = 0;
	while(1) {
		switch(tok.u) {
		case '(':
			readop();
			v = express();
			checkrp();
			checkreg(&v);
			if(tok.u == '+') {
				readop();
				v.val.u |= AMAUTOINCR | t;
				return(v.val.u);
			}
			if(t == 0) {
				v.val.u |= AMDEFERRED;
				return(v.val.u);
			}
			v.val.u |= AMIXDEFER;
			if(padrb - adrbuf > 6)
				addrovf();
			*padrb++ = 0;
			*padrb++ = 0;
			*padrb++ = xsymbol;
			return(v.val.u);

		case '-':
			readop();
			if(tok.u != '(') {
				savop = tok.u;
				tok.u = '-';
				break;
			}
			readop();
			v = express();
			checkrp();
			checkreg(&v);
			v.val.u |= t | AMAUTODECR;
			return(v.val.u);


		case '$':
			readop();
			v = express();
			if(padrb - adrbuf > 6)
				addrovf();
			*padrb++ = v.val.u;
			*padrb++ = v.type.u;
			*padrb++ = xsymbol;
			v.val.u = t | AMIMMED;
			return(v.val.u);

		case '*':
			if(t != 0)
				aerror('*');
			t = AMDEFERRED;
			readop();
			continue;

		default:
			break;
		}
		break;		/* only continue statement loops... */
	}

	v = express();
	if(tok.u == '(') {
		readop();
		if(padrb - adrbuf > 6)
			addrovf();
		*padrb++ = v.val.u;
		*padrb++ = v.type.u;
		*padrb++ = xsymbol;
		v = express();
		checkreg(&v);
		checkrp();
		v.val.u |= AMINDEXED | t;
		return(v.val.u);
	}

	if(v.type.u == TYPEREGIS) {
		checkreg(&v);
		v.val.u |= t;
		return(v.val.u);
	}

	v.type.u |= 0100000;	/* relative address */
	v.val.u -= (dot+4);
	if(padrb != &adrbuf[0])
		v.val.u -= 2;
	if(padrb - adrbuf > 6)
		addrovf();
	*padrb++ = v.val.u;
	*padrb++ = v.type.u;
	*padrb++ = xsymbol;
	return(AMRELATIVE | t);
}



/*
	Routine to "handle" attempt to load more than two adresses
	in adrbuf
*/
void addrovf()
{
	printf("addrovf: address over flow, line %d\n",line);
	aexit(1);
}



/*
	Routine to check that a value is a valid register
*/
void checkreg(v)
	struct value *v;
{
	if(v->val.u > 7 ||
	   (v->type.u > TYPEABS && v->type.u < TYPEOPFD)) {
	   aerror('a');
	   v->val.i = 0;
	   v->type.i = TYPEUNDEF;
	}
}


/*
	Routine to check for an expected right paren
*/
void checkrp()
{
	if(tok.i != ')') {
		aerror(')');
		return;
	}
	readop();
	return;
}


/*
	Routine to set an entry in jmp/br table and return
	the number of types used in the instruction.  A 1 bit
	means that a branch over a jmp must be used.
*/
int setbr(v)
	int v;
{
	if(brtabp > BRLEN)	/* no more room in table... */
		return(2);
	++brtabp;
	if((v -= dot) > 0)
		v -= brdelt;
	if(v >= -254 && v <= 256)	/* in range... */
		return(0);
	brtab[(brtabp-1) >> 3] |= (1 << ((brtabp-1) & 7));	/* out */
	return(2);
}


/*
	Routine to check the current entry in jmp/br table.
	Return of 0 means that a br can be used.
*/
int getbr()
{
	int t;

	if(brtabp > BRLEN)
		return(1);
	t = brtabp++;
	return((brtab[t >> 3] >> (t & 7)) & 1);
}

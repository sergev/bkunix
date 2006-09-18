/*
	as - PDP/11 assembler, Part I

	expression evaluator
*/

#include "as.h"
#include "as1.h"

int combine();

/*
	Routine to parse and evaluate an expression
	Returns value as a type/value structure
*/
struct value express()
{
	struct value v,rv;
	int opfound,ttype;
	char oldop;

	oldop = '+';
	opfound = 0;
	v.val.i = 0;
	v.type.i = TYPEABS;

	while(1) {

		if((rv.type.u = tok.u) >= TOKSYMBOL) {	/* name/opcode */
			rv.type.i = tok.v->type.b;
			rv.val.i = tok.v->val.i;
			goto operand;
		}
		if(tok.u >= FBBASE) {	/* local symbol reference */
			if(tok.u >= FBFWD) {
				v.type.i = TYPEUNDEF;
				v.val.i = 0;
				goto operand;
			}
			rv.type.i = curfbr[tok.i - FBBASE];
			rv.val.i = curfb[tok.i - FBBASE];
			if(v.val.i < 0)
				aerror('f');
			goto operand;
		}

		switch(tok.i) {
		case '+':	case '-':	case '*':	case '/':
		case '&':	case TOKVBAR:	case TOKLSH:
		case TOKRSH:	case '%':	case '^':
		case '!':
			if(oldop != '+')
				aerror('e');
			oldop = tok.u;
			readop();
			continue;

		case TOKINT:
			rv.val.i = numval;
			rv.type.i = TYPEABS;
			break;

		case '[':
			readop();
			rv = express();
			if(tok.u != ']')
				aerror(']');
			break;

		default:
			if(!opfound)
				aerror('e');
			return(v);
		}

	operand:

		++opfound;
		ttype = combine(v.type.i,rv.type.i,0);	/* tentative */
		switch(oldop) {

		case '+':
			v.type.i = ttype;
			v.val.i += rv.val.i;
			break;
		
		case '-':
			v.type.i = combine(v.type.i,rv.type.i,1);
			v.val.i -= rv.val.i;
			break;

		case '*':
			v.type.i = ttype;
			v.val.i *= rv.val.i;
			break;

		case '/':
			v.type.i = ttype;
			v.val.i /= rv.val.i;
			break;

		case TOKVBAR:
			v.type.i = ttype;
			v.val.i |= rv.val.i;
			break;

		case '&':
			v.type.i = ttype;
			v.val.i &= rv.val.i;
			break;

		case TOKLSH:
			v.type.i = ttype;
			v.val.i <<= rv.val.i;
			break;

		case TOKRSH:
			v.type.i = ttype;
			v.val.u >>= rv.val.i;
			break;

		case '%':
			v.type.i = ttype;
			v.val.i %= rv.val.i;
			break;

		case '!':
			v.type.i = ttype;
			v.val.i += ~rv.val.u;
			break;

		case '^':
			v.type.i = rv.type.i;
			break;

		default:
			break;
		}

		oldop = '+';
		readop();
	}
	return(v);	/* dummy... */
}


/*
	Routine to determine type after combining to operands
*/
int combine(left,right,sflag)
int left,right,sflag;
{
	int ext,t;

	ext = (left | right) & TYPEEXT;
	left &= 037;
	right &= 037;
	if(right > left) {				/* highest type on left */
		t = right;
		right = left;
		left = t;
	}
	if(right == TYPEUNDEF)			/* if either one was undef */
		return(ext);
	if(!sflag)						/* not subtract */
		return(left | ext);
	if(left != right)				/* subtract unlike types */
		return(left | ext);
	return(ext | TYPEABS);			/* subtract like types */
}

		

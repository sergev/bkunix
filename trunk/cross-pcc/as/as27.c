/*
	AS - PDP/11 Assembler, Part II

	Expression parsing / evaluation.
*/

#include <stdio.h>
#include "as.h"
#include "as2.h"

struct value express()
{
	struct value expres1();

	xsymbol = 0;
	return(expres1());
}


struct value expres1()
{
	struct value v,rv;
	int oldop;
	struct fb_tab *pfb;

	oldop = '+';
	v.val.i = 0;
	v.type.i = TYPEABS;

	while(1) {
		if(tok.i > TOKSYMBOL) {
			if((rv.type.i = tok.v->type.b) == TYPEUNDEF &&
			   passno != 0)
			   aerror('u');
			if(rv.type.i == TYPEEXT) {
				xsymbol = tok.u;
				rv.val.i = 0;
				goto operand;
			}
			rv.val.i = tok.v->val.i;
			goto operand;
		}

		if(tok.u >= FBBASE) {
			pfb = curfb[tok.u - FBBASE];
			rv.val.i = pfb->val;
			rv.type.i = pfb->rel;
			goto operand;
		}

		switch(tok.u) {

		case '+': case '-': case '*': case '/':
		case '&': case '%': case '^': case '!':
		case TOKVBAR: case TOKLSH: case TOKRSH:
			if(oldop != '+')
				aerror('e');
			oldop = tok.u;
			readop();
			continue;

		case TOKINT:
			agetw();
			rv.val.i = tok.i;
			rv.type.i = TYPEABS;
			goto operand;

		case 2:
			rv.val.i = numval;
			rv.type.i = TYPEABS;
			goto operand;

		case '[':
			readop();
			rv = expres1();
			if(tok.u != ']')
				aerror(']');
			goto operand;

		default:
			return(v);
		}

	operand:

		switch(oldop){

		case '+':
			v.type.i = combine(v.type.i,rv.type.i,reltp2);
			v.val.i += rv.val.i;
			break;

		case '-':
			v.type.i = combine(v.type.i,rv.type.i,reltm2);
			v.val.i -= rv.val.i;
			break;

		case '*':
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i *= rv.val.i;
			break;

		case '/':
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i /= rv.val.i;
			break;

		case TOKVBAR:
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i |= rv.val.i;
			break;

		case '&':
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i &= rv.val.i;
			break;

		case TOKLSH:
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i <<= rv.val.i;
			break;

		case TOKRSH:
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.u >>= rv.val.i;
			break;

		case '%':
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i %= rv.val.i;
			break;

		case '^':
			v.type.i = rv.type.i;
			break;

		case '!':
			v.type.i = combine(v.type.i,rv.type.i,relte2);
			v.val.i += ~rv.val.u;
			break;

		default:
			break;
		}

		oldop = '+';
		readop();
	}
	return(v);	/* never execued - for compiler */
}


/*
	Routine to determine type after an operation
*/
int combine(left,right,table)
int left,right;
int table[6][6];
{
	int t,t2;

	if(passno == 0) {
		t = (right | left) & TYPEEXT;
		right &= 037;
		left &= 037;
		if(right > left) {
			t2 = right;
			right = left;
			left = t2;
		}
		if(right == TYPEUNDEF)
			return(t);
		if(table != &reltm2 || left != right)
			return(t | left);
		return(t | TYPEABS);
	}

	maxtyp = 0;
	left = table[maprel(right)][maprel(left)];
	if(left < 0) {
		if(left != -1)
			aerror('r');
		return(maxtyp);
	}
	return(left);
}



/*
	Routine to map relocation flag and type into reltX2
	table index, and calculate "max" type
*/
int maprel(type)
{
	if(type == TYPEEXT)
		return(5);
	if((type &= 037) > maxtyp)
		maxtyp = type;
	if(type > TYPEOPFD)
		return(1);
	return(type);
}

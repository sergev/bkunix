/*
	as - PDP/11 assembler, Part I

	Statement (Opcode/Operand) Handling and addressing mode parsing
*/

#include <stdio.h>
#include "as.h"
#include "as1.h"

/*
	Routine to process one statement
*/
void opline()
{
	struct value v;
	int t;

	if(tok.u <= TOKSYMBOL) {		/* Operator */
		if(tok.u != '<') {
			express();
			dot += 2;
			return;
		}
		dot += numval;				/* <string> */
		readop();
		return;
	}

	t = tok.v->type.u;
	if(t == TYPEREGIS || t < TYPEOPFD || t > TYPEOPJCC) { /* not op code */
		express();
		dot += 2;
		return;
	}

	readop();
	switch(t) {

	case TYPEOPBR:					/* 1 word instructions */
	case TYPEOPRTS:
	case TYPEOPSYS:
		express();
		dot += 2;
		return;

	case TYPEOPJSR:					/* 2 operand instructions */
	case TYPEOPDO:
	case TYPEOPMOVF:
	case TYPEOPFF:
	case TYPEOPMUL:
		address();
		if(tok.i != ',') {
			aerror('a');
			return;
		}
		readop();
		address();
		dot += 2;
		return;

	case TYPEOPSO:					/* 1 operand instructions */
		address();
		dot += 2;
		return;

	case TYPEOPBYTE:				/* .byte	*/
		while(1) {
			express();
			++dot;
			if(tok.i != ',')
				break;
			readop();
		}
		return;

	case TYPEOPWORD:				/* .word	*/
		while(1) {
			express();
			dot += 2;
			if(tok.i != ',')
				break;
			readop();
		}
		return;

	case TYPEOPASC:					/* <...>	*/
		dot += numval;
		readop();
		return;

	case TYPEOPEVEN:				/* .even	*/
		dot = (dot+1) & ~1;
		return;

	case TYPEOPIF:					/* .if		*/
		v = express();
		if(v.type.i == TYPEUNDEF)
			aerror('U');
		if(v.val.i == 0)
			++ifflg;
		return;

	case TYPEOPEIF:					/* .endif	*/
		return;

	case TYPEOPGLB:					/* .globl	*/
		while(tok.u >= TOKSYMBOL) {
			tok.v->type.u |= TYPEEXT;
			readop();
			if(tok.u != ',')
				break;
			readop();
		}
		return;

	case TYPEREGIS:
	case TYPEOPEST:
	case TYPEOPESD:
		express();
		dot += 2;
		return;

	case TYPEOPTXT:					/* .txt, .data, .bss */
	case TYPEOPDAT:
	case TYPEOPBSS:
		savdot[dotrel-TYPETXT] = dot;
		dot = savdot[t-TYPEOPTXT];
		dotrel = t-TYPEOPTXT+TYPETXT;
		return;

	case TYPEOPSOB:					/* sob		*/
		express();
		if(tok.u != ',')
			aerror('a');
		readop();
		express();
		dot += 2;
		return;

	case TYPEOPCOM:					/* .common	*/
		if(tok.u < TOKSYMBOL) {
			aerror('x');
			return;
		}
		tok.v->type.u |= TYPEEXT;
		readop();
		if(tok.u != ',') {
			aerror('x');
			return;
		}
		readop();
		express();
		return;

	case TYPEOPJBR:					/* jbr		*/
		v = express();
		if(v.type.i != dotrel || (v.val.i -= dot) > 0 || v.val.i < -376)
			dot += 4;
		else
			dot += 2;
		return;

	case TYPEOPJCC:					/* jcc		*/
		v = express();
		if(v.type.i != dotrel || (v.val.i -= dot) > 0 || v.val.i < 376)
			dot += 6;
		else
			dot += 2;
		return;
	default:
		break;
	}

	aerror('~');
	fprintf(stderr,"opline: internal error, line %d\n",line);
	aexit();
}


/*
	Routine to parse an address and return bytes needed
*/
int address()
{
	int i;
	struct value v;

	switch(tok.i) {

	case '(':
		readop();
		v = express();
		checkrp();
		checkreg(v);
		if(tok.i == '+') {
			readop();
			return(0);
		}
		return(2);

	case '-':
		readop();
		if(tok.i != '(') {	/* not really auto decrement */
			savop = tok.i;
			tok.i = '-';
			break;
		}
		readop();
		v = express();
		checkrp();
		checkreg(v);
		return(0);

	case '$':
		readop();
		express();
		dot += 2;
		return(0);

	case '*':
		readop();
		if(tok.i == '*')
			aerror('*');
		i = address();
		dot += i;
		return(i);

	default:
		break;
	}

	v = express();
	if(tok.i == '(') {			/* indexed */
		readop();
		v = express();
		checkreg(v);
		checkrp();
		dot += 2;
		return(0);
	}
	if(v.type.i == TYPEREGIS) {
		checkreg(v);
		return(0);
	}
	dot += 2;
	return(0);
}


/*
	Routine to check that a value is in range for a register
*/
void checkreg(v)
	struct value v;
{
	if(v.val.u > 7 || (v.type.u != TYPEABS && v.type.u <= TYPEBSS))
		aerror('a');
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
}

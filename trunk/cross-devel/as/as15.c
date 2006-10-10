/*
	as - PDP/11 assembler, Part I

	Main scanner routine and some subroutines
*/

#include "as.h"
#include "as1.h"

/*
	Main scanner routine
*/
void readop()
{
	unsigned char c;
	int i;

	if((tok.i = savop) != 0) {
		savop = 0;
		return;
	}

	while(1) {
		while((c = chartab[tok.i = rch()]) == CHARWHITE)
			;
		if(c != 0 && c < 128)
			goto rdname;

		switch(c) {

		case CHARSTRING:	/* <...>	*/
			tok.i = '<';
			aputw();
			numval = 0;
			while(c = rsch(), !eos_flag) {
				tok.i = c | STRINGFLAG;
				aputw();
				++numval;
			}
			tok.i = -1;
			aputw();
			tok.i = '<';
			return;

		case CHARLF:		/* character is the token */
		case CHARTOKEN:
			break;

		case CHARSKIP:		/* / - comment */
			while((c = rch()) != TOKEOF && c != '\n')
				;
			tok.i = c;		/* newline or eof */
			break;

		case CHARNAME:
		rdname:
			ch = tok.i;
			if(c < '0' || c > '9') {
				rname(c);
				return;
			}
			/* fall thru since it is a number */

		case CHARNUM:
			if(!number())
				break;		/* really a temporary label */
			numval = num_rtn;
			tok.i = TOKINT;
			aputw();
			tok.i = numval;
			aputw();
			tok.i = TOKINT;
			return;

		case CHARSQUOTE:
		case CHARDQUOTE:
			if(c == CHARSQUOTE)
				numval = rsch();
			else {
				numval = rsch();
				numval |= rsch() << 8;
			}
			tok.i = TOKINT;
			aputw();
			tok.i = numval;
			aputw();
			tok.i = TOKINT;
			return;

		case CHARGARB:
			aerror('g');
			continue;

		case CHARESCP:
			c = rch();
			for(i=0; esctab[i] != 0; i+=2) {
				if(esctab[i] == c) {
					tok.i = esctab[i+1];
					break;
				}
			}
			break;

		case CHARFIXOR:
			tok.i = TOKVBAR;
			break;
		}

		aputw();
		return;
	}
}


/*
	Routine to read a character inside a string ( <...> )
*/
char rsch()
{
	char c;
	int i;

	if((c = rch()) == TOKEOF || c == '\n') {
		aerror('<');
		aexit();
	}
	eos_flag = 0;
	if(c == '\\') {
		c = rch();
		for(i = 0; schar[i] != 0; i += 2) {
			if(schar[i] == c)
				return(schar[i+1]);
		}
		return(c);
	}
	eos_flag = (c == '>');
	return(c);
}

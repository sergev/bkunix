/*
 * as - PDP/11 Assember, Part I
 *
 * Main assembly control routine
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as1.h"

void assem()
{
	struct value v;
	union token savtok;
	int i;

	while(1) {
		readop();
		if(checkeos())
			goto ealoop;
		if(ifflg) {					/* Inside .if */
			if(tok.u <= TOKSYMBOL)
				continue;
			if(tok.v->type.i == TYPEOPIF)
				++ifflg;
			if(tok.v->type.i == TYPEOPEIF)
				--ifflg;
			continue;
		}

		savtok.i = tok.i;
		readop();
		if(tok.i == '=') {
			readop();
			v = express();
			if(savtok.u < TOKSYMBOL) {
				aerror('x');
				goto ealoop;
			}
			/* coerced to int * */
			if((int*)tok.s == &dotrel) {
				v.type.u &= ~TYPEEXT;
				if(v.type.i != dotrel) {
					aerror('.');
					dotrel = TYPETXT;
					goto ealoop;
				}
			}
			savtok.v->type.u &= ~037;
			v.type.u &= 037;
			if(v.type.u == TYPEUNDEF)
				v.val.i = 0;
			savtok.v->type.u |= v.type.u;
			savtok.v->val.i = v.val.i;
			goto ealoop;
		}  /* = */

		if(tok.i == ':') {
			tok.i = savtok.i;
			if(tok.u >= TOKSYMBOL) {
				if(tok.v->type.u & 037)
					aerror('m');
				tok.v->type.u |= dotrel;
				tok.v->val.i = dot;
				continue;
			}
			if(tok.i != TOKINT) {
				aerror('x');
				continue;
			}
			i = fbcheck(numval);	/* n: */
			curfbr[i] = dotrel;
			nxtfb.label = i << 8 | dotrel;
			nxtfb.val = dot;
			curfb[i] = dot;
			write_fb(fbfil, &nxtfb);
			continue;
		}	/* : */

		savop = tok.i;
		tok.i = savtok.i;
		opline();

ealoop:

		if(tok.i == ';')
			continue;
		if(tok.i == '\n') {
			++line;
			continue;
		}
		if(tok.i != TOKEOF) {
			aerror('x');
			while(!checkeos())
				readop();
			continue;
		}
		if(ifflg)
			aerror('x');
		return;
	}
}

void write_fb(f, b)
	int f;
	struct fb_tab *b;
{
	char buf[4];

	buf[0] = b->label;
	buf[1] = b->label >> 8;
	buf[2] = b->val;
	buf[3] = b->val >> 8;
	if(write(f, buf, 4) != 4)
		fprintf(stderr,"assem: error writing to fb file.\n");
}


/*
	Routine to check a number to see if it is in range for
	a temporary label
*/
unsigned fbcheck(u)
	unsigned u;
{
	if(u > 9) {
		aerror('f');
		u = 0;
	}
	return(u);
}


/*
	Routine to check current token to see if we are at the end of
	a statement
*/
int checkeos()
{
	return(tok.i == '\n' || tok.i == ';' || tok.i == '#' || tok.i == TOKEOF);
}

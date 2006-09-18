/*
	AS - PDP/11 Assembler, Part II

	Main assembly control routine
*/

#include <stdio.h>
#include "as.h"
#include "as2.h"

void assem()
{
	union token ttok;

	while(1) {
		readop();
		if(tok.u != TOKFILE && tok.u != '<') {
			if(checkeos())
				goto ealoop;
			ttok.u = tok.u;
			if(tok.u == TOKINT) {
				ttok.u = 2;
				agetw();
				numval = tok.u;
			}
			readop();
			switch(tok.u) {
			case '=':
				doequal(&ttok);
				goto ealoop;
			case ':':
				docolon(&ttok);
				continue;
			default:
				savop = tok.u;
				tok.u = ttok.u;
				break;
			}
		}

		opline();
		dotmax();

	ealoop:

		if(tok.u == '\n')
			++line;
		if(DEBUG)
			printf("\nLine %d: ",line);
		if(tok.u == TOKEOF)
			return;
	}
}


/*
	Subroutine to process assignment  (label = value)
*/
void doequal(t)
union token *t;
{
	struct value v;
	int i;
	struct value express();

	readop();
	v = express();
	if(t->v == &symtab) {	/* .= */
		v.type.u &= ~TYPEEXT;
		if(v.type.u != dotrel) {
			aerror('.');
			return;
		}
		if(v.type.u == TYPEBSS) {
			dot = v.val.u;
			dotmax();
			return;
		}
		v.val.u -= dot;
		if(v.val.i < 0) {
			aerror('.');
			return;
		}
		for(i = v.val.i-1; i >= 0; --i)
			outb(TYPEABS,0);
		dotmax();
		return;
	}

	if(v.type.u == TYPEEXT)
		aerror('r');
	t->v->type.u &= ~037;
	v.type.u &= 037;
	if(v.type.u == TYPEUNDEF)
		v.val.u = 0;
	t->v->type.b = v.type.u;
	t->v->val.u = v.val.u;
}


/*
	Subroutine to handle a label definition
*/
void docolon(t)
union token *t;
{
	unsigned ttype;

	tok.u = t->u;
	if(tok.u < TOKSYMBOL) {
		if(tok.u != 2) {
			aerror('x');
			return;
		}
		tok.u = numval;
		fbadv();
		curfb[tok.u]->rel = dotrel;
		brdelt = curfb[tok.u]->val - dot;
		curfb[tok.u]->val = dot;
		return;
	}

	if(passno == 0) {
		ttype = tok.v->type.b & 037;
		if(ttype != 0 &&
			(ttype < TYPEOPEST || ttype > TYPEOPESD) )
			aerror('m');
		tok.v->type.b &= ~037;
		tok.v->type.b |= dotrel;
		brdelt = tok.v->val.u - dot;
		tok.v->val.u = dot;
		return;
	}

	if(dot != tok.v->val.u)
		aerror('p');
	return;
}


/*
	Routine to check if token marks end of statement
*/
int checkeos()
{
	return(tok.u == '\n' || tok.u == ';' || tok.u == TOKEOF);
}


/*
	Routine to advance fb file (or reset)

	tok has number to work on
*/
void fbadv()
{
	struct fb_tab *p;

	p = curfb[tok.i] = nxtfb[tok.i];
	if(p == 0)
		p = fbbufp;
	else
		++p;
	while(p->lblix != tok.i && !(p->lblix & 0200)) {
		++p;
	}
	if(DEBUG)
		printf("fbadv %d to %o %o ",tok.i,p->rel,p->val);
	nxtfb[tok.i] = p;
}


/*
	Routine to track . high water mark for text, data and bss
*/
void dotmax()
{
	if(passno == 0 && dot > hdr.atxtsiz[dotrel-TYPETXT])
		hdr.atxtsiz[dotrel-TYPETXT] = dot;
	return;
}

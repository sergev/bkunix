/*
 * AS - PDP/11 Assembler, Part II
 *
 * Main assembly control routine
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as2.h"

void p2_assem(struct pass2 *p2)
{
	union token ttok;

	while(1) {
		p2_readop(p2);
		if(p2->tok.u != TOKFILE && p2->tok.u != '<') {
			if(p2_checkeos(p2))
				goto ealoop;
			ttok.u = p2->tok.u;
			if(p2->tok.u == TOKINT) {
				ttok.u = 2;
				p2_agetw(p2);
				p2->numval = p2->tok.u;
			}
			p2_readop(p2);
			switch(p2->tok.u) {
			case '=':
				p2_doequal(p2, &ttok);
				goto ealoop;
			case ':':
				p2_docolon(p2, &ttok);
				continue;
			default:
				p2->savop = p2->tok.u;
				p2->tok.u = ttok.u;
				break;
			}
		}

		p2_opline(p2);
		p2_dotmax(p2);

	ealoop:

		if(p2->tok.u == '\n')
			++p2->line;
		if(DEBUG)
			printf("\nLine %d: ", p2->line);
		if(p2->tok.u == TOKEOF)
			return;
	}
}


/*
	Subroutine to process assignment  (label = value)
*/
void p2_doequal(struct pass2 *p2, union token *t)
{
	struct value v;
	int i;

	p2_readop(p2);
	v = p2_express(p2);
	if(t->v == &p2->symtab[0]) {	/* .= */
		v.type.u &= ~TYPEEXT;
		if(v.type.u != dotrel(p2)) {
			p2_aerror(p2, '.');
			return;
		}
		if(v.type.u == TYPEBSS) {
			p2->symtab[0].val.u = v.val.u;
			p2_dotmax(p2);
			return;
		}
		v.val.u -= dot(p2);
		if(v.val.i < 0) {
			p2_aerror(p2, '.');
			return;
		}
		for(i = v.val.i-1; i >= 0; --i)
			p2_outb(p2, TYPEABS,0);
		p2_dotmax(p2);
		return;
	}

	if(v.type.u == TYPEEXT)
		p2_aerror(p2, 'r');
	t->v->type.u &= ~037;
	v.type.u &= 037;
	if(v.type.u == TYPEUNDEF)
		v.val.u = 0;
	t->v->type.u |= v.type.u;
	t->v->val.u = v.val.u;
}


/*
	Subroutine to handle a label definition
*/
void p2_docolon(struct pass2 *p2, union token *t)
{
	unsigned ttype;

	p2->tok.u = t->u;
	if(p2->tok.u < TOKSYMBOL) {
		if(p2->tok.u != 2) {
			p2_aerror(p2, 'x');
			return;
		}
		p2->tok.u = p2->numval;
		p2_fbadv(p2);
		p2->curfb[p2->tok.u]->label &= ~0xff;
		p2->curfb[p2->tok.u]->label |= dotrel(p2);
		p2->brdelt = p2->curfb[p2->tok.u]->val - dot(p2);
		p2->curfb[p2->tok.u]->val = dot(p2);
		return;
	}

	if(p2->passno == 0) {
		ttype = p2->tok.v->type.u & 037;
		if(ttype != 0 &&
		    (ttype < TYPEOPEST || ttype > TYPEOPESD))
			p2_aerror(p2, 'm');
		p2->tok.v->type.u &= ~037;
		p2->tok.v->type.u |= dotrel(p2);
		p2->brdelt = p2->tok.v->val.u - dot(p2);
		p2->tok.v->val.u = dot(p2);
		return;
	}

	if(dot(p2) != p2->tok.v->val.u)
		p2_aerror(p2, 'p');
	return;
}


/*
	Routine to check if token marks end of statement
*/
int p2_checkeos(struct pass2 *p2)
{
	return(p2->tok.u == '\n' || p2->tok.u == ';' || p2->tok.u == TOKEOF);
}


/*
	Routine to advance fb file (or reset)

	tok has number to work on
*/
void p2_fbadv(struct pass2 *p2)
{
	struct fb_tab *p;

	p = p2->curfb[p2->tok.i] = p2->nxtfb[p2->tok.i];
	if(p == 0)
		p = p2->fbbufp;
	else
		++p;
	while((p->label >> 8) != p2->tok.i && !(p->label & ENDTABFLAG)) {
		++p;
	}
	if(DEBUG)
		printf("fbadv %ld to %o %o ", (long)p2->tok.i, (char) p->label, p->val);
	p2->nxtfb[p2->tok.i] = p;
}


/*
	Routine to track . high water mark for text, data and bss
*/
void p2_dotmax(struct pass2 *p2)
{
	if(p2->passno == 0 && dot(p2) > p2->hdr.atxtsiz[dotrel(p2)-TYPETXT])
		p2->hdr.atxtsiz[dotrel(p2)-TYPETXT] = dot(p2);
	return;
}

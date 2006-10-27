/*
 * as - PDP/11 Assembler, Part I
 *
 * Scanner subroutines
 * Symbol table subroutines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as1.h"

/*
	Routine to read a name whose 1st character is
	contained in variable c
*/
void rname(c)
	unsigned char c;
{
	char *sp;
	unsigned char tc;
	unsigned short hv, next;
	struct value *stok;
	int no_hash, sl, probe;
	char debug = 0;

	sl = 8;
	no_hash = 0;
	memset(symbol, 0, 8);
	sp = symbol;
	if(c == '~') {
		++no_hash;
		ch = 0;
	}
	while((c = chartab[tc = rch()]) < 128 && c != 0) {
		if(--sl >= 0)
			*sp++ = c;
	}
	hv = hash(symbol);
	ch = tc;
	if(no_hash) {
		tok.s = symend;
		add_symbol(tok.s, symbol);
	}
	else {
		probe = hv % HSHSIZ;
		next = (hv / HSHSIZ) + 1;
		while(1) {
			if((probe -= next) < 0)
				probe += HSHSIZ;
			if(debug)
				printf("rname debug probe %d next %u\n",probe,next);
			if(hshtab[probe] == 0) {
				hshtab[probe] = tok.s = symend;
				add_symbol(tok.s, symbol);
				break;
			}
			if(debug)
				printf("rname debug comparing to %s\n",hshtab[probe]->name);
			if(strncmp(hshtab[probe]->name,symbol,8) == 0) {
				tok.s = hshtab[probe];
				break;
			}
		}
	}

	stok = &tok.s->v;
	if(tok.s >= usymtab)
		tok.i = (tok.s - usymtab) + USYMFLAG;
	else
		tok.i = (tok.s - symtab) + PSYMFLAG;
	aputw();
	tok.v = stok;
}


/*
	Routine to handle numbers and temporary labels
	Numbers starting from 0 are treated as octal.
*/
char number()
{
	int num, base;
	unsigned char c;

	if ((c = rch()) != '0') {
		base = 10;
		ch = c;
	} else if ((c = rch()) != 'x' && c != 'X') {
		base = 8;
		ch = c;
	} else {
		base = 16;
	}
	num = 0;
	for (;;) {
		c = rch();
		if (c >= '0' && c <= '7')
			c -= '0';
		else if (base >= 10 && c >= '8' && c <= '9')
			c -= '0';
		else if (base == 16 && c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (base == 16 && c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		num = num * base + c;
	}
	if(c != 'b' && c != 'f') {
		num_rtn = num;
		ch = c;
		return(TRUE);
	}
	/*
		Temporary label reference
	*/
	tok.i = fbcheck(num) + (c == 'b' ? FBBASE : FBFWD);
	return(FALSE);
}


/*
	Routine to read next character
	Uses character routines so that MSDOS crlf turns into lf
*/
unsigned char rch()
{
	int c;
	int savtok;
	char *pc;

	if((c = ch) != 0) {
		ch = 0;
		return(c);
	}
	while(TRUE) {
		if(fin != 0) {
			if((c = fgetc(fin)) != EOF)
				return(c & 0x7f);
		}
		if(fin != 0)
			fclose(fin);
		if(--nargs <= 0)
			return(TOKEOF);
		if(ifflg) {
			aerror('i');
			aexit();
		}
		++fileflg;
		if((fin = fopen(*++curarg,"r")) == NULL) {
			filerr(*curarg,"rch: can't open file.");
			aexit();
		}
		line = 1;
		savtok = tok.i;
		tok.i = TOKFILE;
		aputw();
		for(pc = *curarg; *pc != '\0'; ++pc) {
			tok.i = *pc;
			aputw();
		}
		tok.i = -1;
		aputw();
		tok.i = savtok;
	}
}


/*
	Routine to hash a symbol and enter into hash table
*/
void hash_enter(p)
	struct symtab *p;
{
	unsigned short hv, next;
	int probe;
	char debug = 0;

	hv = hash(p->name);
	probe = hv % HSHSIZ;
	next = (hv / HSHSIZ) + 1;
	while(TRUE) {
		if((probe -= next) < 0)
			probe += HSHSIZ;
		if(debug)
			printf("hash_enter: probe %d next %u\n",probe,next);
		if(hshtab[probe] == 0) {
			hshtab[probe] = p;
			break;
		}
	}
}


/*
	Routine to hash a symbol
*/
unsigned short hash(p)
	char *p;
{
	int i;

	unsigned h = 0;
	for(i = 0; i < 8 && p[i] != '\0'; ++i) {
		h += p[i];
		h = (h  << 8) | ((h >> 8) & 0xFF);
	}
	return(h);
}


/*
	Routine to add a symbol to the symbol table and bump
	the symbol table pointer
*/
void add_symbol(p, s)
	struct symtab *p;
	char *s;
{
	strncpy(p->name, s, 8);
	if(++symend - usymtab > USERSYMBOLS) {
		fprintf(stderr,"add_symbol: symbol table overflow.\n");
		aexit();
	}
}

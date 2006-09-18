/*
	AS - PDP/11 Assembler, Part II

	More miscellaneous support routines
*/

#include <stdio.h>
#include "as.h"
#include "as2.h"

/*
	Routine to set up a buffered file, with an initial offset
*/
void oset(p, o)
	struct out_buf *p;
	int o;
{
	p->slot = (int*) ((char*)p + (o & 0777) + 6);
	p->max = (int*) ((char*)p->buf + sizeof p->buf);
	p->seek = o;
	if(DEBUG)
		printf("oset offset %x slot %d seek %d ",
			o,p->slot - p->buf,p->seek);
}


/*
	Routine to write a word to a buffered file
*/
void aputw(p, v)
	struct out_buf *p;
	int v;
{
	int *pi;

	if((pi = p->slot) < p->max) {
		*pi++ = v;
		p->slot = pi;
	}
	else {
		flush(p);
		*(p->slot++) = v;
	}
	if(DEBUG)
		printf("aputw  %s %o slot %d ",
			(p == &relp) ? "rel" : "txt",v,p->slot - p->buf);
}


/*
	Routine to flush a buferred file
*/
void flush(p)
	struct out_buf *p;
{
	char *wb;
	int wc;

	if(DEBUG)
		printf("flush seek to %x ",p->seek);
	lseek(fout,(long)p->seek,0);
	wb = (char *)&p->buf + (p->seek & 0777);
	p->seek = (p->seek | 0777) + 1;
	wc = (char *)p->slot - wb;
	p->slot = (int*) &p->buf;
	write(fout,wb,wc);
}


/*
	Routine to read a token from the token file created in
	the first pass
*/
void readop()
{
	tok.i = savop;
	if(tok.i != 0) {
		savop = 0;
		return;
	}
	agetw();
	if(tok.u > TOKSYMBOL) {
		if(tok.u >= USYMFLAG) {
			tok.u -= USYMFLAG;
			tok.v = &usymtab[tok.u];
		}
		else {
			tok.u -= PSYMFLAG;
			tok.v = &symtab[tok.u];
		}
	}
}


/*
	Routine to read a word from token file created in pass 1
*/
int agetw()
{
	if( (tok.u = savop) ) {
		savop = 0;
		return(TRUE);
	}
	if(read(fin,&tok.u,2) < 2) {
		tok.u = TOKEOF;
		return(FALSE);
	}
	return(TRUE);
}

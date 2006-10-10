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
	p->slot = p->buf + (o & 0777);
	p->max = p->buf + sizeof p->buf;
	p->seek = o;
	if(DEBUG)
		printf("\noset: offset %x slot %d seek %d\n",
			o, (p->slot - p->buf) / 2, p->seek);
}


/*
	Routine to write a word to a buffered file
*/
void aputw(p, v)
	struct out_buf *p;
	int v;
{
	char *pi;

	pi = p->slot;
	if(pi < p->max) {
		*pi++ = v;
		*pi++ = v >> 8;
		p->slot = pi;
	}
	else {
		flush(p);
		*(p->slot++) = v;
		*(p->slot++) = v >> 8;
	}
	if(DEBUG)
		printf("aputw  %s %o slot %d ", (p == &relp) ? "rel" : "txt",
			v, (p->slot - p->buf) / 2);
}


/*
	Routine to flush a buferred file
*/
void flush(p)
	struct out_buf *p;
{
	char *addr;
	int bytes;

	addr = p->buf + (p->seek & 0777);
	bytes = p->slot - addr;

	if(DEBUG)
		printf("\nflush: write %d bytes, seek to %x\n", bytes, p->seek);
	if (bytes == 0)
		return;
	lseek(fout, (long)p->seek, 0);
	write(fout, addr, bytes);

	p->seek = (p->seek | 0777) + 1;
	p->slot = p->buf;
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
	unsigned char buf[2];

	tok.u = savop;
	if(tok.u != 0) {
		savop = 0;
		return(TRUE);
	}
	if(read(fin, buf, 2) < 2) {
		tok.u = TOKEOF;
		return(FALSE);
	}
	tok.u = buf[0] | buf[1] << 8;
	return(TRUE);
}

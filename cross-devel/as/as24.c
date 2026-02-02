/*
 * AS - PDP/11 Assembler, Part II
 *
 * More miscellaneous support routines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as2.h"

/*
	Routine to set up a buffered file, with an initial offset
*/
void p2_oset(struct pass2 *p2, struct out_buf *p, int o)
{
	(void)p2;
	p->slot = p->buf + (o & 0777);
	p->max = p->buf + sizeof p->buf;
	p->seek = o;
	if(DEBUG)
		printf("\noset: offset %x slot %d seek %d\n",
			o, (int) (p->slot - p->buf) / 2, p->seek);
}


/*
	Routine to write a word to a buffered file
*/
void p2_aputw(struct pass2 *p2, struct out_buf *p, int v)
{
	char *pi;

	pi = p->slot;
	if(pi < p->max) {
		*pi++ = v;
		*pi++ = v >> 8;
		p->slot = pi;
	}
	else {
		p2_flush(p2, p);
		*(p->slot++) = v;
		*(p->slot++) = v >> 8;
	}
	if(DEBUG)
		printf("aputw  %s %o slot %d ", (p == &p2->relp) ? "rel" : "txt",
			v, (int) (p->slot - p->buf) / 2);
}


/*
	Routine to flush a buferred file
*/
void p2_flush(struct pass2 *p2, struct out_buf *p)
{
	char *addr;
	int bytes;

	addr = p->buf + (p->seek & 0777);
	bytes = p->slot - addr;

	if(DEBUG)
		printf("\nflush: write %d bytes, seek to %x\n", bytes, p->seek);
	if (bytes == 0)
		return;
	lseek(p2->fout, (long)p->seek, 0);
	write(p2->fout, addr, bytes);

	p->seek = (p->seek | 0777) + 1;
	p->slot = p->buf;
}


/*
	Routine to read a token from the token file created in
	the first pass
*/
void p2_readop(struct pass2 *p2)
{
	p2->tok.i = p2->savop;
	if(p2->tok.i != 0) {
		p2->savop = 0;
		return;
	}
	p2_agetw(p2);
	if(p2->tok.u > TOKSYMBOL) {
		if(p2->tok.u >= USYMFLAG) {
			p2->tok.u -= USYMFLAG;
			p2->tok.v = &p2->usymtab[p2->tok.u];
		}
		else {
			p2->tok.u -= PSYMFLAG;
			p2->tok.v = &p2->symtab[p2->tok.u];
		}
	}
}


/*
	Routine to read a word from token file created in pass 1
*/
int p2_agetw(struct pass2 *p2)
{
	unsigned char buf[2];

	p2->tok.u = p2->savop;
	if(p2->tok.u != 0) {
		p2->savop = 0;
		return(TRUE);
	}
	if(read(p2->fin, buf, 2) < 2) {
		p2->tok.u = TOKEOF;
		return(FALSE);
	}
	p2->tok.u = buf[0] | buf[1] << 8;
	return(TRUE);
}

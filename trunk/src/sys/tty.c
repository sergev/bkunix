/*
 * General TTY subroutines.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "tty.h"

#define CROUND 7

/*
 * The character lists-- space for 6*NCLIST characters if lucky,
 * or 6 less otherwise: getc/putc needs the structs to be 8-aligned;
 * cinit() takes care of that.
 */
struct cblock cfree[NCLIST];

/* List head for unused character blocks. */
struct cblock *cfreelist;

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
void
cinit()
{
	register struct cblock *cp;

	cp = (struct cblock*) (((int)cfree + CROUND) & ~CROUND);
	for (; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
}

/*
 * Character list get/put
 */
int
getc(p)
	register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;

	s = spl7();
	if (p->c_cf == 0) {
		c = -1;
		p->c_cc = 0;
		p->c_cf = p->c_cl = 0;
	} else {
		c = *p->c_cf++ & 0377;
		if (--p->c_cc <= 0) {
			bp = (struct cblock*) ((int)(p->c_cf - 1) & ~CROUND);
			p->c_cf = p->c_cl = 0;
			goto reclaim;
		}
		if (((int)p->c_cf & CROUND) == 0) {
			bp = (struct cblock*) p->c_cf - 1;
			p->c_cf = bp->c_next->c_info;
reclaim:		bp->c_next = cfreelist;
			cfreelist = bp;
		}
	}
	splx(s);
	return c;
}

int
putc(c, p)
	int c;
	register struct clist *p;
{
	register struct cblock *bp;
	register char *cp;
	register s;

	s = spl7();
	cp = p->c_cl;
	if (cp == 0) {
		bp = cfreelist;
		if (bp == 0) {
eof:			splx(s);
			return -1;
		}
		p->c_cf = bp->c_info;
		goto alloc;
	}
	if (((int)cp & CROUND) == 0) {
		bp = (struct cblock*) cp - 1;
		bp->c_next = cfreelist;
		if (bp->c_next == 0)
			goto eof;
		bp = bp->c_next;
alloc:		cfreelist = bp->c_next;
		bp->c_next = 0;
		cp = bp->c_info;
	}
	*cp++ = c;
	p->c_cc++;
	p->c_cl = cp;
	splx(s);
	return 0;
}

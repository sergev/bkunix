/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	maintain date
 *	alarm clock signals
 */
#ifdef CLOCKOPT
static unsigned rem, last;

struct timer {
	int initval;
	int curval;
	int flags;
};

#define TIMER ((struct timer *)0177706)

void clkinit()
{
	/* reprime if accidentally stopped */
	TIMER->flags = 0162;
	rem = 0;
	last = TIMER->curval;
}

/* incr cannot be greater than 178 (65535*128/46875) */
void clock(incr)
	register unsigned incr;
{
	register struct proc *pp;

	time += incr;
	for(pp = &proc[0]; pp < &proc[NPROC]; pp++) {
		if(pp->p_clktim) {
			if(pp->p_clktim <= incr) {
				pp->p_clktim = 0;
				psignal(pp, SIGCLK);
			} else {
				pp->p_clktim -= incr;
			}
		}
	}
}

#define DENOM (unsigned)46875

void uptime()
{
	register unsigned quot = 0, tmp = 0;
	register unsigned cur = TIMER->curval;
	register int power;

	if (cur == last)
		return;

	/* the hardware register keeps decrementing */
	cur = last-cur;
	last -= cur;

/* computing (128*cur + rem) / DENOM, calling clock with quotient
 * and updating rem.
 */
	tmp = rem >> 7;
       	/*tmp >>= 7;*/	/* PCC goofs here, remove next stmt when fixed */
	tmp &= 511;
	if (cur >= DENOM - tmp) {
		/* Normalize to proper fraction */
		cur -= DENOM - tmp;
       		quot = 128;
		rem &= 127;
	} else cur += tmp;
	rem <<= 9;	/* bring MSB of rem to bit 15 for easier testing */
	power = 64;
	do {
		tmp = (int) rem < 0;
		if (cur >= (DENOM - tmp + 1)>>1) {
			/* the fraction is >= 0.5, next bit of quot is 1 */
			cur = (cur<<1) + tmp - DENOM;
			quot += power;
		} else {
			/* next bit of quot is 0 */
			cur = (cur<<1) + tmp;
		}
		rem <<= 1;
	} while (power >>= 1);
	rem = cur;
	if (quot)
		clock(quot);
}

#endif

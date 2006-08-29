#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
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
 *	maintain user/system times
 *	maintain date
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 */
#ifdef CLOCKOPT
clock(dev, sp, r1, nps, r0, pc, ps)
char *pc;
{
	register struct proc *pp;

	/*
	 * restart clock
	 */

#ifdef CLOCK
	CLOCK->integ = 0115;
#endif

	/*
	 * lightning bolt time-out
	 * and time of day
	 */

	if(pc > TOPSYS) {
		u.u_utime++;
	} else
		u.u_stime++;
	if(++lbolt >= HZ) {
		lbolt =- HZ;
		if(++time[1] == 0)
			++time[0];
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++) {
			if(pp->p_clktim)
				if(--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
		}
		if(pc > TOPSYS) {
			u.u_ar0 = &r0;
			if(issig())
				psig();
		}
	}
}
#endif

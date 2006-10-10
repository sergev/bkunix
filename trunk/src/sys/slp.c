/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "user.h"
#include "proc.h"
#include "systm.h"
#include "file.h"
#include "inode.h"
#include "buf.h"
#ifdef BGOPTION
#include "tty.h"
#endif

#ifdef BGOPTION
int swflg, swwait;
#endif

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process resumes execution.
 * The most important effect of pri is that when
 * pri<=0 a signal cannot disturb the sleep;
 * if pri>0 signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
void
sleep(chan, pri)
	int chan, pri;
{
	register struct proc *rp;
	register int s;

	rp = u.u_procp;
	s = spl7();
#ifdef BGOPTION
	rp->p_stat = ((pri == PRIBIO) || (pri == TTOPRI)) ? SWAIT : SSLEEP;
#endif
#ifndef BGOPTION
	rp->p_stat = SSLEEP;
#endif
	rp->p_wchan = chan;
	spl0();
	if(pri >= 0) {
		if(issig())
			goto psig;
#ifdef BGOPTION
		swtch(1);
#endif
#ifndef BGOPTION
		idle();
#endif
		if(issig())
			goto psig;
	} else
#ifdef BGOPTION
		swtch(1);
#endif
#ifndef BGOPTION
		idle();
#endif
	rstps(s);
	return;

	/*
	 * If priority was low (>0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */
psig:
	rp->p_stat = SRUN;
	retu(u.u_qsav);
}

/*
 * Wake up process if sleeping on chan.
 */
void
wakeup(chan)
	int chan;
{
	register struct proc *p;

#ifdef BGOPTION
	for(p = &proc[0]; p < &proc[NPROC+2]; p++) {
		if(p->p_wchan == chan)
			setrun(p);
	}
#endif
#ifndef BGOPTION
	p = &proc[cpid];
	if(p->p_wchan == chan)
		setrun(p);
#endif
}

/*
 * Set the process running;
 */
void
setrun(p)
	struct proc *p;
{
	register struct proc *rp;

	rp = p;
	rp->p_wchan = 0;
	rp->p_stat = SRUN;
}


/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process.
 * How this happens is rather hard to understand.
 * The essential fact is that the new process is created
 * in such a way that appears to have started executing
 * in the same call to newproc as the parent;
 * but in fact the code that runs is that of swtch.
 * The subtle implication of the returned value of swtch
 * (see above) is that this is the value that newproc's
 * caller in the new process sees.
 */
int
newproc()
{
	register struct proc *rpp;
	register struct file **rip, *fp;

	/*
	 * make duplicate entries
	 * where needed
	 */
	for(rip = &u.u_ofile[0]; rip < &u.u_ofile[NOFILE];) {
		fp = *rip++;
		if(fp != NULL)
			fp->f_count++;
	}
	u.u_cdir->i_count++;
	savu(u.u_ssav);	/* save state of parent */
#ifdef BGOPTION
	retu(u.u_rsav);
#endif
	rpp = u.u_procp;
	rpp->p_stat = SIDL;
#ifdef BGOPTION
	swap(B_WRITE, cpid);
#endif
#ifndef BGOPTION
	swap(B_WRITE);
#endif
	cpid++;
	rpp = &proc[cpid];
	rpp->p_stat = SRUN;
	u.u_procp = rpp;
#ifdef BGOPTION
	retu(u.u_ssav);
#endif
	return(1);	/* return to child */
}

#ifdef BGOPTION
swtch(aa)
{
	register struct proc *p,*p1;
	register s;
	static int a;

	if(bgproc == 0) {
		if(aa) idle();
		return;
	}
	if(issig())
		psig();
	s = spl7();
	if(swflg) {
		swwait = 1;
		rstps(s);
		return;
	}
	swflg = 1;
	rstps(s);
	a = aa;
loop:	p = &proc[cpid];
	if(p->p_stat == SSLEEP) {
		p1 = bgproc;
		if(p1->p_stat == SRUN) p = p1;
		else {
			idle();
			swflg = 0;
			return;
		}
	}
	p1 = u.u_procp;
	if((p == p1) || (cpid == (NPROC-1))) {
		swflg = 0;
		return;
	}
	savu(u.u_ssav);
	retu(u.u_rsav);
	u.u_procp = &proc[NPROC+1];
	s = (p == bgproc) ? NPROC : cpid;
	swap(B_WRITE,(s == cpid) ? NPROC : cpid);
	swap(B_READ,s);
	retu(u.u_ssav);
	u.u_procp = &proc[s];
	setrun(p);
	s = spl7();
	if(swwait) {
		swwait = 0;
		rstps(s);
		goto loop;
	}
	swflg = 0;
	swwait = 0;
	rstps(s);
}
#endif

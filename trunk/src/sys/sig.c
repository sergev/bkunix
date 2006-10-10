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
#include "inode.h"
#include "reg.h"

/*
 * Send the specified signal to
 * all processes with 'tp' as its
 * controlling teletype.
 * Called by tty.c for quits and
 * interrupts.
 */
void
signal(sig)
	int sig;
{
	register struct proc *p;

	for(p = &proc[0]; p < &proc[NPROC]; p++)
		psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
void
psignal(p, sig)
	struct proc *p;
	int sig;
{
	register struct proc *rp;

	if(sig >= NSIG)
		return;
	rp = p;
	rp->p_sig = sig;
	if(rp->p_stat == SSLEEP)
		setrun(rp);
	if(user && issig())
		psig();
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
int
issig()
{
	register int n;
	register struct proc *p;

	p = u.u_procp;
	n = p->p_sig;
	if(n != 0) {
		if((u.u_signal[n]&1) == 0)
			return(n);
	}
	return(0);
}

/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 * It writes USIZE block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
int
core()
{
	register struct inode *ip;

	u.u_error = 0;
	u.u_dirp = "core";
	u.u_segflg++;
	ip = namei(1);
	if(ip == NULL) {
		if(u.u_error)
			return(0);
		ip = maknode(0666);
		if (ip == NULL)
			return(0);
	}
	itrunc(ip);
	u.u_offset[0] = 0;
	u.u_offset[1] = 0;
	u.u_base = (char*) &u;
	u.u_count = UCORE+USIZE;
	writei(ip);
	iput(ip);
	u.u_segflg--;
	return(u.u_error==0);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if(issig())
 *		psig();
 */
void
psig()
{
	register int n, p;
	register struct proc *rp;

	rp = u.u_procp;
	n = rp->p_sig;
	rp->p_sig = 0;
	p = u.u_signal[n];
	if(p != 0) {
		u.u_error = 0;
		if(n != SIGINS)
			u.u_signal[n] = 0;
		n = u.u_ar0[R6] - 4;
		if (! bad_user_address ((char*) n)) {
			*(int*)(n+2) = u.u_ar0[RPS];
			*(int*)n = u.u_ar0[R7];
		}
		u.u_ar0[R6] = n;
		u.u_ar0[RPS] &= ~TBIT;
		u.u_ar0[R7] = p;
		return;
	}
	switch(n) {

	case SIGQIT:
	case SIGINS:
	case SIGTRC:
	case SIGIOT:
	case SIGEMT:
	case SIGFPT:
	case SIGBUS:
	case SIGSEG:
	case SIGSYS:
		u.u_arg[0] = n;
		if(core())
			n += 0200;
	}
	u.u_arg[0] = (u.u_ar0[R0]<<8) | n;
	pexit();
}

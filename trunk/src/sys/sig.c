#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "inode.h"
#include "reg.h"

/*
 * Structure to access an array of integers.
 */
struct
{
	int	inta[];
};

/*
 * Send the specified signal to
 * all processes with 'tp' as its
 * controlling teletype.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(sig)
{
	register struct proc *p;

	for(p = &proc[0]; p < &proc[NPROC]; p++)
		psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
int *p;
char *sig;
{
	register *rp;
	extern user;

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
issig()
{
	register n;
	register struct proc *p;

	p = u.u_procp;
	if(n = p->p_sig) {
		if((u.u_signal[n]&1) == 0)
			return(n);
	}
	return(0);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if(issig())
 *		psig();
 */
psig()
{
	register n, p;
	register *rp;

	rp = u.u_procp;
	n = rp->p_sig;
	rp->p_sig = 0;
	if((p=u.u_signal[n]) != 0) {
		u.u_error = 0;
		if(n != SIGINS)
			u.u_signal[n] = 0;
		n = u.u_ar0[R6] - 4;
		suword(n+2, u.u_ar0[RPS]);
		suword(n, u.u_ar0[R7]);
		u.u_ar0[R6] = n;
		u.u_ar0[RPS] =& ~TBIT;
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
			n =+ 0200;
	}
	u.u_arg[0] = (u.u_ar0[R0]<<8) | n;
	exit();
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
core()
{
	register s, *ip;

	u.u_error = 0;
	u.u_dirp = "core";
	ip = namei(1);
	if(ip == NULL) {
		if(u.u_error)
			return(0);
		ip = maknode(0666);
		if (ip==NULL)
			return(0);
	}
	itrunc(ip);
	u.u_offset[0] = 0;
	u.u_offset[1] = 0;
/*
	u.u_base = &u;
	u.u_count = USIZE*64;
	writei(ip);
*/
	u.u_base = &u;
	u.u_count = (UCORE+USIZE)*64;
	writei(ip);
	iput(ip);
	return(u.u_error==0);
}

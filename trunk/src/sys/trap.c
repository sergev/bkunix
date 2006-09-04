/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "reg.h"

#define	EBIT	1		/* user error bit in PS: C-bit */
#define	SETD	0170011		/* SETD instruction */
#define	SYS	0104400		/* sys (trap) instruction */
#define	USER	020		/* user-mode flag added to dev */

/*
 * Call the system-entry routine f (out of the
 * sysent table). This is a subroutine for trap, and
 * not in-line, because if a signal occurs
 * during processing, an (abnormal) return is simulated from
 * the last caller to savu(qsav); if this took place
 * inside of trap, it wouldn't have a chance to clean up.
 *
 * If this occurs, the return takes place without
 * clearing u_intflg; if it's still set, trap
 * marks an error which means that a system
 * call (like read on a typewriter) got interrupted
 * by a signal.
 */
static void
trap1(f)
	int (*f)();
{
	u.u_intflg = 1;
	savu(u.u_qsav);
	(*f)();
	u.u_intflg = 0;
}

/*
 * Called from l40.s or l45.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
void
trap(dev, sp, r1, nps, r0, pc, ps)
	char *pc;
{
	register i, a;
	register struct sysent *callp;

	if(pc > (char*) TOPSYS)
		dev |= USER;
	u.u_ar0 = &r0;
	switch(dev) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	default:
		panic();

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if(fuiword(pc-2) == SETD && u.u_signal[SIGINS] == 0)
			goto out;
		i = SIGINS;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

	case 5+USER: /* emt */
		i = SIGEMT;
		break;

	case 6+USER: /* sys call */
		u.u_error = 0;
		ps &= ~EBIT;
		callp = &sysent[fuiword(pc-2)&077];
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc += 2;
			i = fuword(a);
			if ((i & ~077) != SYS)
				i = 077;	/* illegal */
			callp = &sysent[i&077];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a += 2);
		} else {
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc += 2;
			}
		}
		u.u_dirp = (char*) u.u_arg[0];
		trap1(callp->call);
		if(u.u_intflg)
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				ps |= EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
		i = SIGSYS;
		break;

	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */
		psignal(u.u_procp, SIGFPT);
		return;

	case 8+USER:
		i = SIGFPT;
		break;

	}
	psignal(u.u_procp, i);

out:
	if(issig())
		psig();
}

/*
 * nonexistent system call-- set fatal error code.
 */
void
nosys()
{
	u.u_error = 100;
}

/*
 * Ignored system call
 */
void
nullsys()
{
}

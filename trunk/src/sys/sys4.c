#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

/*
 * Everything in this file is a routine implementing a system call.
 */

#include "param.h"
#include "user.h"
#include "reg.h"
#include "inode.h"
#include "systm.h"
#include "proc.h"

gtime()
{

	u.u_ar0[R0] = time[0];
	u.u_ar0[R1] = time[1];
}

stime()
{

	time[0] = u.u_ar0[R0];
	time[1] = u.u_ar0[R1];
	wakeup(tout);
}

getuid()
{

	u.u_ar0[R0] = 0;
}

getpid()
{
	u.u_ar0[R0] = cpid;
}

sync()
{

	update();
}

/*
 * Unlink system call.
 */
unlink()
{
	register *ip, *pp;

	pp = namei(2);
	if(pp == NULL)
		return;
	ip = iget(pp->i_dev, u.u_dent.u_ino);
	if(ip == NULL)
		goto out1;
	u.u_offset[1] =- DIRSIZ+2;
	u.u_base = &u.u_dent;
	u.u_count = DIRSIZ+2;
	u.u_dent.u_ino = 0;
	writei(pp);
	ip->i_nlink--;
	ip->i_flag =| IUPD;

out:
	iput(ip);
out1:
	iput(pp);
}

chdir()
{
	register *ip;
	extern uchar;

	ip = namei(0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
	bad:
		iput(ip);
		return;
	}
	if(access(ip, IEXEC))
		goto bad;
	iput(u.u_cdir);
	u.u_cdir = ip;
}

chmod()
{
	register *ip;

	if ((ip = owner()) == NULL)
		return;
	ip->i_mode =& ~07777;
	ip->i_mode =| u.u_arg[1]&07777;
	ip->i_flag =| IUPD;
	iput(ip);
}

ssig()
{
	register a;

	a = u.u_arg[0];
	if(a<=0 || a>=NSIG) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ar0[R0] = u.u_signal[a];
	u.u_signal[a] = u.u_arg[1];
	u.u_signal[9] = 0;		/* kill not allowed */
	if(u.u_procp->p_sig == a)
		u.u_procp->p_sig = 0;
}

/*
 * alarm clock signal
 */
alarm()
{
	register c, *p;

	p = u.u_procp;
	c = p->p_clktim;
	p->p_clktim = u.u_ar0[R0];
	u.u_ar0[R0] = c;
}

/*
 * indefinite wait.
 * no one should wakeup(&u)
 */

pause()
{

	for(;;)
		sleep(&u, PSLEP);
}

/*
 * become the background process
 */

#ifdef BGOPTION
bground()
{
	register int *p1,*p2;
	if(bgproc) {
		u.u_error = EAGAIN;
		return;
	}
	p1 = &proc[NPROC];
	p2 = u.u_procp;
	while(p1 < &proc[NPROC+1]) *p1++ = *p2++;
	spl7();
	bgproc = &proc[NPROC];
	cpid--;
	u.u_procp = bgproc;
	spl0();
}

kill()
{
	if(bgproc == 0) {
		u.u_error = ESRCH;
		return;
	}
	psignal(bgproc,9);
}
#endif

#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "buf.h"
#include "reg.h"
#include "inode.h"

/*
 * exec system call.
 * Because of the fact that an I/O buffer is used
 * to store the caller's arguments during exec,
 * and more buffers are needed to read in the text file,
 * deadly embraces waiting for free buffers are possible.
 * Therefore the number of processes simultaneously
 * running in exec has to be limited to NEXEC.
 */
#define EXPRI	-1

exec()
{
	int ap, na, nc, *bp;
	int ds;
	register c, *ip;
	register char *cp;
	int *ptr;

	/*
	 * pick up file names
	 * and check various modes
	 * for execute permission
	 */

	ip = namei(0);
	if(ip == NULL)
		return;
	bp = getblk(NODEV);
	if(access(ip, IEXEC) || (ip->i_mode&IFMT)!=0)
		goto bad;

	/*
	 * pack up arguments into
	 * allocated disk buffer
	 */

	cp = bp->b_addr;
	na = 0;
	nc = 0;
	while(ap = fuword(u.u_arg[1])) {
		na++;
		if(ap == -1)
			goto bad;
		u.u_arg[1] =+ 2;
		for(;;) {
			c = fubyte(ap++);
			if(c == -1)
				goto bad;
			*cp++ = c;
			nc++;
			if(nc > 510) {
				u.u_error = E2BIG;
				goto bad;
			}
			if(c == 0)
				break;
		}
	}
	if((nc&1) != 0) {
		*cp++ = 0;
		nc++;
	}

	/*
	 * read in first 8 bytes
	 * of file for segment
	 * sizes:
	 * w0 = 407
	 * w1 = text size
	 * w2 = data size
	 * w3 = bss size
	 */

	u.u_base = &u.u_arg[0];
	u.u_count = 8;
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	readi(ip);
	if(u.u_error)
		goto bad;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] =+ u.u_arg[1];
	} else {
		u.u_error = ENOEXEC;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */

	ds = ((u.u_arg[2]+u.u_arg[3]+63)>>6) & 01777;
	if(ds + SSIZE > UCORE)
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */

	cp = TOPSYS;
	while(cp < TOPUSR)
		*cp++ = 0;

	/*
	 * read in data segment
	 */

	u.u_base = TOPSYS;
	u.u_offset[1] = 020;
	u.u_count = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */

	u.u_dsize = ds;
	u.u_ssize = SSIZE;
	cp = bp->b_addr;
	ap = TOPUSR - nc - na*2 - 4;
	u.u_ar0[R6] = ap;
	suword(ap, na);
	c = TOPUSR - nc;
	while(na--) {
		suword(ap=+2, c);
		do
			subyte(c++, *cp);
		while(*cp++);
	}
	suword(ap+2, -1);

	/*
	 * clear sigs, regs and return
	 */

	c = ip;
	for(ip = &u.u_signal[0]; ip < &u.u_signal[NSIG]; ip++)
		if((*ip & 1) == 0)
			*ip = 0;
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[R7] = TOPSYS;
#ifndef BGOPTION
	for(ip = &u.u_fsav[0]; ip < &u.u_fsav[25];)
		*ip++ = 0;
#endif
	ip = c;

bad:
	iput(ip);
	brelse(bp);
}

/*
 * exit system call:
 * pass back caller's r0
 */
rexit()
{

	u.u_arg[0] = u.u_ar0[R0] << 8;
	exit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit()
{
	register int *q, a;
	register struct proc *p;
#ifdef BGOPTION
	extern swflg,swwait;
#endif

	p = u.u_procp;
	p->p_clktim = 0;
	for(q = &u.u_signal[0]; q < &u.u_signal[NSIG];)
		*q++ = 1;
	for(q = &u.u_ofile[0]; q < &u.u_ofile[NOFILE]; q++)
		if(a = *q) {
			*q = NULL;
			closef(a);
		}
	iput(u.u_cdir);
	update();
#ifdef BGOPTION
	if(p != bgproc) {
		q = getblk(SWAPDEV, SWPLO+swtab[cpid].sw_blk);
		bcopy(&u, q->b_addr, 256);
		bwrite(q);
		if(cpid)
			cpid--;
		else
			panic();
	}
	if(p == bgproc) bgproc = 0;
	retu(u.u_rsav);
	u.u_procp = &proc[NPROC+1];
	swflg = 1;
	swap(B_READ,cpid);
	spl7();
	swflg = 0;
	swwait = 0;
	spl0();
	u.u_procp = &proc[cpid];
#endif
#ifndef BGOPTION
	q = getblk(SWAPDEV, SWPLO+cpid*SWPSIZ);
	bcopy(&u, q->b_addr, 256);
	bwrite(q);
	if(cpid)
		cpid--;
	else
		panic();
	retu(u.u_rsav);	/* switch to system stack */
	swap(B_READ);
#endif
	retu(u.u_ssav);
#ifdef BGOPTION
	setrun(&proc[cpid]);
#endif
	p->p_stat = SZOMB;
	return(0);	/* return to parent */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * NOTE: if cpid == NPROC then the status read in is erroneous
 */
wait()
{
	register *bp;
	register struct proc *p;
	register chpid;

	p = &proc[chpid = cpid+1];
	if(p->p_stat == SZOMB) {
		u.u_ar0[R0] = chpid;
#ifdef BGOPTION
		bp = bread(SWAPDEV, SWPLO+swtab[chpid].sw_blk);
#endif
#ifndef BGOPTION
		bp = bread(SWAPDEV, SWPLO+chpid*SWPSIZ);
#endif
		p->p_stat = NULL;
		p->p_sig = 0;
		p = bp->b_addr;
		u.u_cstime[0] =+ p->u_cstime[0];
		dpadd(u.u_cstime, p->u_cstime[1]);
		dpadd(u.u_cstime, p->u_stime);
		u.u_cutime[0] =+ p->u_cutime[0];
		dpadd(u.u_cutime, p->u_cutime[1]);
		dpadd(u.u_cutime, p->u_utime);
		u.u_ar0[R1] = p->u_arg[0];
		brelse(bp);
		return;
	} else
		u.u_error = ECHILD;
}

/*
 * fork system call.
 */
fork()
{

#ifdef BGOPTION
	if((cpid == NPROC-1) || (u.u_procp == bgproc)) {
#endif
#ifndef BGOPTION
	if(cpid == NPROC-1) {
#endif
		u.u_error = EAGAIN;
		goto out;
	}
	if(newproc()) {
		u.u_ar0[R0] = cpid;
		u.u_cstime[0] = 0;
		u.u_cstime[1] = 0;
		u.u_stime = 0;
		u.u_cutime[0] = 0;
		u.u_cutime[1] = 0;
		u.u_utime = 0;
		return;
	}
	u.u_ar0[R0] = cpid+1;

out:
	u.u_ar0[R7] =+ 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	register a, n, d;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = (((u.u_arg[0]-TOPSYS+63)>>6) & 01777);
	d = n - u.u_dsize;
	n =+ USIZE+u.u_ssize;
	if(n > UCORE) {
		u.u_error = E2BIG;
		return;
	}
	u.u_dsize =+ d;
}

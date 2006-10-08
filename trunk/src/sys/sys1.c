/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "buf.h"
#include "reg.h"
#include "inode.h"
#include "file.h"

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

void
exec()
{
	int na, nc, ds;
	struct buf *bp;
	register int c;
	register struct inode *ip;
	register char *cp;
	int *sp;
	char *up;

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
	for (;;) {
		up = *(char**) u.u_arg[1];
		if (up == 0)
			break;
		if (bad_user_address (up))
			goto bad;
		na++;
		u.u_arg[1] += 2;
		for(;;) {
			if (bad_user_address (up))
				goto bad;
			c = *up++;
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
	u.u_base = (char*) &u.u_arg[0];
	u.u_count = 8;
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	u.u_segflg++;
	readi(ip);
	u.u_segflg--;
	if(u.u_error)
		goto bad;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] += u.u_arg[1];
	} else {
		u.u_error = ENOEXEC;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */
	ds = u.u_arg[2]+u.u_arg[3];
	c = nc + na*2 + 4;
#ifdef LOWSTACK
	ds += c;
#endif
	if(ds + SSIZE > UCORE)
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */
	memzero ((void*) BOTUSR, TOPUSR - BOTUSR);

	/*
	 * read in data segment
	 */
	u.u_base = (char*) BOTUSR;
	u.u_offset[1] = 020;
	u.u_count = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */
	u.u_dsize = ds;
	u.u_ssize = SSIZE;
	cp = bp->b_addr;
#ifdef LOWSTACK
	u.u_ar0[R6] = BOTUSR + ds - c;
	up = (char*) (ds - nc);
#else
	u.u_ar0[R6] = TOPUSR - c;
	up = (char*) (TOPUSR - nc);
#endif
	sp = (int*) u.u_ar0[R6];
	*sp++ = na;
	while(na--) {
		*sp++ = (int) up;
		do {
			*up++ = *cp;
		} while(*cp++);
	}
	*sp = 0;

	/*
	 * clear sigs, regs and return
	 */
	for(sp = &u.u_signal[0]; sp < &u.u_signal[NSIG]; sp++)
		if((*sp & 1) == 0)
			*sp = 0;
	u.u_ar0[R0] = 0;
	u.u_ar0[R1] = 0;
	u.u_ar0[R2] = 0;
	u.u_ar0[R3] = 0;
	u.u_ar0[R4] = 0;
	u.u_ar0[R5] = 0;
	u.u_ar0[R7] = BOTUSR;
bad:
	iput(ip);
	brelse(bp);
}

/*
 * exit system call:
 * pass back caller's r0
 */
void
rexit()
{
	u.u_arg[0] = u.u_ar0[R0] << 8;
	pexit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
void
pexit()
{
	register int *q;
	register struct file **fp;
	register struct proc *p;
	struct buf *bp;

	p = u.u_procp;
	p->p_clktim = 0;
	for(q = &u.u_signal[0]; q < &u.u_signal[NSIG];)
		*q++ = 1;
	for(fp = &u.u_ofile[0]; fp < &u.u_ofile[NOFILE]; fp++)
		if(*fp != NULL) {
			closef(*fp);
			*fp = NULL;
		}
	iput(u.u_cdir);
	update();
#ifdef BGOPTION
	if(p != bgproc) {
		bp = getblk(SWAPDEV, SWPLO+swtab[cpid].sw_blk);
		memcpy(bp->b_addr, &u, 512);
		bwrite(bp);
		if(cpid)
			cpid--;
		else
			panic("zero pid");
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
	bp = getblk(SWAPDEV, SWPLO+cpid*SWPSIZ);
	memcpy(bp->b_addr, &u, 512);
	bwrite(bp);
	if(cpid)
		cpid--;
	else
		panic("zero pid");
	retu(u.u_rsav);	/* switch to system stack */
	swap(B_READ);
#endif
	retu(u.u_ssav);
#ifdef BGOPTION
	setrun(&proc[cpid]);
#endif
	p->p_stat = SZOMB;
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * NOTE: if cpid == NPROC then the status read in is erroneous
 */
void
wait()
{
	register struct buf *bp;
	struct proc *p;
	register struct user *q;
	register int chpid;

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
		q = (struct user*) bp->b_addr;
		u.u_ar0[R1] = q->u_arg[0];
		brelse(bp);
		return;
	} else
		u.u_error = ECHILD;
}

/*
 * fork system call.
 */
void
sfork()
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
		return;
	}
	u.u_ar0[R0] = cpid+1;
out:
	u.u_ar0[R7] += 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
void
sbreak()
{
	register int n, d;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */
	n = ((u.u_arg[0]+1)&~1)-BOTUSR;
#ifndef LOWSTACK
	d = n - u.u_dsize;
	n += USIZE+u.u_ssize;
#endif
	if(n > UCORE) {
		u.u_error = E2BIG;
		return;
	}
	u.u_dsize += d;
}

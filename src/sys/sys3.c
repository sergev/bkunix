#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "systm.h"
#include "reg.h"
#include "buf.h"
#include "filsys.h"
#include "user.h"
#include "inode.h"
#include "file.h"

/*
 * the fstat system call.
 */
fstat()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, u.u_arg[0]);
}

/*
 * the stat system call.
 */
stat()
{
	register ip;

	ip = namei(0);
	if(ip == NULL)
		return;
	stat1(ip, u.u_arg[1]);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
int *ip;
{
	register i, *bp, *cp;

	iupdat(ip);
	bp = bread(ip->i_dev, (ip->i_number+31)/16);
	cp = bp->b_addr + 32*((ip->i_number+31)&017) + 24;
	ip = &(ip->i_dev);
	for(i=0; i<14; i++) {
		suword(ub, *ip++);
		ub =+ 2;
	}
	for(i=0; i<4; i++) {
		suword(ub, *cp++);
		ub =+ 2;
	}
	brelse(bp);
}

/*
 * the dup system call.
 */
dup()
{
	register i, *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if ((i = ufalloc()) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

stty()
{
	klsgtty(1);
}

gtty()
{
	klsgtty(0);
}

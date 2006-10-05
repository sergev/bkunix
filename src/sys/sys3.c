/*
 * Copyright 1975 Bell Telephone Laboratories Inc
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
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
static void
stat1(ip, ub)
	struct inode *ip;
	int *ub;
{
	register int i, *cp;
	register struct buf *bp;

	if (bad_user_address (ub) || bad_user_address (ub + 18))
		return;
	iupdat(ip);
	bp = bread(ip->i_dev, (ip->i_number + 31) >> 4);
	cp = &(ip->i_dev);
	for(i=0; i<14; i++) {
		*ub++ = *cp++;
	}
	cp = (int*) (bp->b_addr + 32 * ((ip->i_number + 31) & 017) + 24);
	for(i=0; i<4; i++) {
		*ub++ = *cp++;
	}
	brelse(bp);
}

/*
 * the fstat system call.
 */
void
fstat()
{
	register struct file *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, u.u_arg[0]);
}

/*
 * the stat system call.
 */
void
stat()
{
	register struct inode *ip;

	ip = namei(0);
	if(ip == NULL)
		return;
	stat1(ip, u.u_arg[1]);
	iput(ip);
}

/*
 * the dup system call.
 */
void
dup()
{
	register int i;
	register struct file *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if ((i = ufalloc()) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

void
stty()
{
	klsgtty(1);
}

void
gtty()
{
	klsgtty(0);
}

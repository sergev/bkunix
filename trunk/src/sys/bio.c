#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "user.h"
#include "buf.h"
#include "systm.h"
#include "proc.h"

/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char	buffers[NBUF][512];
struct	buf	swbuf;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no on else can touch it.  If the block was
 * already in core, no I/O need be done.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
bread(dev, blkno)
{
	register struct buf *rbp;

	rbp = getblk(dev, blkno);
	if (rbp->b_flags&B_DONE)
		return(rbp);
	rbp->b_flags =| B_READ;
	rbp->b_wcount = -256;
	fdstrategy(rbp);
	iowait(rbp);
	return(rbp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	rbp->b_wcount = -256;
	fdstrategy(rbp);
	iowait(rbp);
	brelse(rbp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =| B_DELWRI | B_DONE;
	brelse(rbp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
struct buf *bp;
{
	bp->b_flags =& ~B_BUSY;
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 * When a 512-byte area is wanted for some random reason
 * (e.g. during exec, for the user arglist) getblk can be called
 * with device NODEV to avoid unwanted associativity.
 */
getblk(dev, blkno)
{
	register struct buf *bp;
	register int *bdp, i;
	int *bdp2;

	bdp = NULL;
	for(i = 0; i < NBUF; i++) {
		bp = bufp[i];
		if((bp->b_flags&B_BUSY) == 0)
			bdp = &bufp[i];
		if(dev == NODEV)
			continue;
		if((bp->b_blkno == blkno) && (bp->b_dev == dev)) {
			bdp = &bufp[i];
			goto fnd;
		}
	}
	if(bdp == NULL)
		panic();
	bp = *bdp;
	if(bp->b_flags&B_DELWRI)
		bwrite(bp);
	bp->b_flags = 0;
	bp->b_blkno = blkno;
	bp->b_dev = dev;
fnd:
	bdp2 = bdp+1;
	while(bdp > &bufp[0])
		*--bdp2 = *--bdp;
	*bdp = bp;
	bp->b_flags =| B_BUSY;
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	spl7();
	while ((rbp->b_flags&B_DONE)==0)
		sleep(rbp, PRIBIO);
	spl0();
	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}

/*
 * Zero the core associated with a buffer.
 */
clrbuf(bp)
int *bp;
{
	register *p;
	register c;

	p = bp->b_addr;
	c = 256;
	do
		*p++ = 0;
	while (--c);
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register int i;

	for(i = 0; i < NBUF; i++) {
		bp = bufp[i] = &buf[i];
		bp->b_dev = NODEV;
		bp->b_addr = &buffers[i];
	}
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */

bflush(dev)
{
	register struct buf *bp;
	register int i;

	for(bp = &buf[0]; bp < &buf[NBUF]; bp++)
		if((bp->b_flags&B_DELWRI) && (dev == NODEV||dev==bp->b_dev))
			bwrite(bp);
}

/*
 * swap I/O
 */

#define	USTACK	(TOPSYS-12)
struct { int *intp;};
struct { char *chrp;};

#ifdef BGOPTION
struct swtab swtab[] {
	49*256,	0,
	49*256,	49,
	1*256,	49+49,
	49*256,	50+49,
};

swap(rdflg,tab)
{
	register struct swtab *t;

	swbuf.b_flags = B_BUSY | rdflg;
	swbuf.b_dev = SWAPDEV;
	t = &swtab[tab];
	swbuf.b_wcount = -t->sw_size;
	swbuf.b_blkno = t->sw_blk + SWPLO;
	swbuf.b_addr = &u;
	fdstrategy(&swbuf);
	spl7();
	while((swbuf.b_flags&B_DONE)==0)
		sleep(&swbuf, PSWP);
	spl0();
	return(swbuf.b_flags&B_ERROR);
}
#endif

#ifndef BGOPTION
swap(rdflg)
{
	register int *p, *p1, *p2;

	p = &proc[cpid];
	if(rdflg == B_WRITE) {
		p1 = USTACK->integ;
		p2 = TOPSYS + (u.u_dsize<<6) + (p1.integ&077);
		if(p2 <= p1) {
			p->p_size = u.u_dsize + USIZE +
			    ((TOPUSR>>6)&01777) - ((p1.integ>>6)&01777);
			while(p1.chrp < TOPUSR)
				*p2++ = *p1++;
		} else
			p->p_size = SWPSIZ<<3;
	}
	swbuf.b_flags = B_BUSY | rdflg;
	swbuf.b_dev = SWAPDEV;
	swbuf.b_wcount = -(((p->p_size+7)&~07)<<5);	/* 32 words per block */
	swbuf.b_blkno = SWPLO+cpid*SWPSIZ;
	swbuf.b_addr = &u;
	fdstrategy(&swbuf);
	spl7();
	while((swbuf.b_flags&B_DONE)==0)
		sleep(&swbuf, PSWP);
	spl0();
	if(rdflg == B_READ) {
		p1 = TOPUSR;
		p2 = (p->p_size<<6) + TOPSYS - (USIZE<<6);
		if(p2 <= p1)
			while(p1 >= USTACK->integ.intp)
				*--p1 = *--p2;
	}
	return(swbuf.b_flags&B_ERROR);
}
#endif

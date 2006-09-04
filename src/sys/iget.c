/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "systm.h"
#include "user.h"
#include "inode.h"
#include "filsys.h"
#include "buf.h"

struct inode	inode[NINODE];

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printf warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
struct inode *
iget(dev, ino)
	int dev;
	int ino;
{
	register struct inode *p;
	struct inode *freeip;
	register struct buf *bp;
	register struct mount *mp;
loop:
	freeip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
		if(dev==p->i_dev && ino==p->i_number) {
			if((p->i_flag&IMOUNT) != 0) {
				for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
					if(mp->m_inodp == p) {
						dev = mp->m_dev;
						ino = ROOTINO;
						goto loop;
					}
				panic();
			}
			p->i_count++;
			return(p);
		}
		if(freeip==NULL && p->i_count==0)
			freeip = p;
	}
	p = freeip;
	if(p == NULL) {
		u.u_error = ENFILE;
		return(NULL);
	}
	p->i_dev = dev;
	p->i_number = ino;
	p->i_count++;
	bp = bread(dev, (ino+31)>>4);

	/* Check I/O errors */
	if (bp->b_flags&B_ERROR) {
		brelse(bp);
		iput(p);
		return(NULL);
	}
	memcpy(&p->i_mode, bp->b_addr + 32 * ((ino + 31) & 017), 32-8);
	brelse(bp);
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
void
iput(p)
	struct inode *p;
{
	register struct inode *rp;

	rp = p;
	if(rp->i_count == 1) {
		if(rp->i_nlink <= 0) {
			itrunc(rp);
			rp->i_mode = 0;
			ifree(rp->i_dev, rp->i_number);
		}
		iupdat(rp);
		rp->i_flag = 0;
		rp->i_number = 0;
	}
	rp->i_count--;
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If either is on, update the inode
 * with the corresponding dates
 * set to the argument tm.
 */
void
iupdat(p)
	struct inode *p;
{
	register struct inode *rp;
	register struct buf *bp;
	register int *idata;
	int i;

	rp = p;
	if((rp->i_flag&IUPD) != 0) {
		rp->i_flag &= ~IUPD;
		i = rp->i_number+31;
		bp = bread(rp->i_dev, i/16);
		idata = (int*) (bp->b_addr + 32 * (i & 017));
		memcpy (idata, &rp->i_mode, 32-8);
		idata += 14;
		*idata++ = time[0];
		*idata = time[1];
		bwrite(bp);
	}
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
void
itrunc(ip)
	struct inode *ip;
{
	register struct inode *rp;
	register struct buf *bp;
	register int *cp, *bnop;

	rp = ip;
	if((rp->i_mode&(IFCHR&IFBLK)) != 0)
		return;

	for(bnop = &rp->i_addr[7]; bnop >= &rp->i_addr[0]; bnop--) {
		if(*bnop == 0)
			continue;
		if((rp->i_mode&ILARG) != 0) {
			bp = bread(rp->i_dev, *bnop);
			cp = (int*) (bp->b_addr + 510);
			for(; cp >= (int*) bp->b_addr; cp--)
				if(*cp)
					free(rp->i_dev, *cp);
			brelse(bp);
		}
		free(rp->i_dev, *bnop);
		*bnop = 0;
	}
	rp->i_mode &= ~(ILARG|ICONT);
	rp->i_size0 = 0;
	rp->i_size1 = 0;
	rp->i_flag |= IUPD;
}

/*
 * Make a new file.
 */
struct inode *
maknode(mode)
	int mode;
{
	register struct inode *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
		return(NULL);
	ip->i_flag |= IUPD;
	ip->i_mode = mode | IALLOC;
	ip->i_nlink = 1;
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 */
void
wdir(ip)
	struct inode *ip;
{
	u.u_dent.u_ino = ip->i_number;
	memcpy(&u.u_dent.u_name[0], &u.u_dbuf[0], DIRSIZ);
	u.u_count = DIRSIZ + 2;
	u.u_base = (char*) &u.u_dent;
	writei(u.u_pdir);
	iput(u.u_pdir);
}

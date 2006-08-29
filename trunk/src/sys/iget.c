#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "inode.h"
#include "filsys.h"
#include "buf.h"

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
iget(dev, ino)
{
	register struct inode *p;
	register *ip2;
	int *ip1;
	register struct mount *ip;

loop:
	ip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
		if(dev==p->i_dev && ino==p->i_number) {
			if((p->i_flag&IMOUNT) != 0) {
				for(ip = &mount[0]; ip < &mount[NMOUNT]; ip++)
				if(ip->m_inodp == p) {
					dev = ip->m_dev;
					ino = ROOTINO;
					goto loop;
				}
				panic();
			}
			p->i_count++;
			return(p);
		}
		if(ip==NULL && p->i_count==0)
			ip = p;
	}
	if((p=ip) == NULL) {
		u.u_error = ENFILE;
		return(NULL);
	}
	p->i_dev = dev;
	p->i_number = ino;
	p->i_count++;
	ip = bread(dev, (ino+31)>>4);
	/*
	 * Check I/O errors
	 */
	if (ip->b_flags&B_ERROR) {
		brelse(ip);
		iput(p);
		return(NULL);
	}
	ip1 = ip->b_addr + 32*((ino+31)&017);
	ip2 = &p->i_mode;
	while(ip2 < &p->i_addr[8])
		*ip2++ = *ip1++;
	brelse(ip);
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(p)
struct inode *p;
{
	register *rp;

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
iupdat(p)
int *p;
{
	register *ip1, *ip2, *rp;
	int *bp, i;

	rp = p;
	if((rp->i_flag&IUPD) != 0) {
		rp->i_flag =& ~IUPD;
		i = rp->i_number+31;
		bp = bread(rp->i_dev, i/16);
		ip1 = bp->b_addr + 32*(i&017);
		ip2 = &rp->i_mode;
		while(ip2 < &rp->i_addr[8])
			*ip1++ = *ip2++;
		ip1 =+ 2;
		*ip1++ = time[0];
		*ip1++ = time[1];
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
itrunc(ip)
int *ip;
{
	register *rp, *bp, *cp;
	int *dp, *ep, i;

	rp = ip;
	if((rp->i_mode&(IFCHR&IFBLK)) != 0)
		return;
#ifdef	CONTIG
	if(rp->i_mode&ICONT) {
		for(i = rp->i_addr[0]; i < rp->i_addr[0]+rp->i_addr[1]; i++)
			free(rp->i_dev, i);
		rp->i_addr[0] = rp->i_addr[1] = 0;
		goto cont;
	}
#endif
	for(ip = &rp->i_addr[7]; ip >= &rp->i_addr[0]; ip--)
	if(*ip) {
		if((rp->i_mode&ILARG) != 0) {
			bp = bread(rp->i_dev, *ip);
			for(cp = bp->b_addr+510; cp >= bp->b_addr; cp--)
			if(*cp)
				free(rp->i_dev, *cp);
			brelse(bp);
		}
		free(rp->i_dev, *ip);
		*ip = 0;
	}
cont:
	rp->i_mode =& ~(ILARG|ICONT);
	rp->i_size0 = 0;
	rp->i_size1 = 0;
	rp->i_flag =| IUPD;
}

/*
 * Make a new file.
 */
maknode(mode)
{
	register *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
		return(NULL);
	ip->i_flag =| IUPD;
	ip->i_mode = mode|IALLOC;
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
wdir(ip)
int *ip;
{
	register char *cp1, *cp2;

	u.u_dent.u_ino = ip->i_number;
	cp1 = &u.u_dent.u_name[0];
	for(cp2 = &u.u_dbuf[0]; cp2 < &u.u_dbuf[DIRSIZ];)
		*cp1++ = *cp2++;
	u.u_count = DIRSIZ+2;
	u.u_base = &u.u_dent;
	writei(u.u_pdir);
	iput(u.u_pdir);
}

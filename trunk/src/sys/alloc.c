/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "systm.h"
#include "filsys.h"
#include "buf.h"
#include "inode.h"
#include "user.h"

/*
 * iinit is called once (from main)
 * very early in initialization.
 * It reads the root's super block
 * and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super
 * block. Usually because of an IO error.
 */
void iinit()
{
	register struct buf *bp, *cp;
	register struct filsys *fs;

	bp = bread(ROOTDEV, 1);
	cp = getblk(NODEV, 0);
	if(u.u_error)
		panic("error reading superblock");
	memcpy(cp->b_addr, bp->b_addr, 512);
	brelse(bp);
	mount[0].m_bufp = cp;
	mount[0].m_dev = ROOTDEV;
	fs = (struct filsys*) cp->b_addr;
	time[0] = fs->s_time[0];
	time[1] = fs->s_time[1];
}

/*
 * mount user file system
 */
void minit()
{
	register struct buf *bp, *cp;
	register struct inode *ip;

	bp = bread(MNTDEV, 1);
	if(bp->b_flags&B_ERROR)
		goto nomount;
	u.u_dirp = "/usr";
	u.u_segflg++;
	cp = getblk(NODEV);
	memcpy(cp->b_addr, bp->b_addr, 512);
	ip = namei(0);
	u.u_segflg--;
	mount[1].m_inodp = ip;
	mount[1].m_dev = MNTDEV;
	mount[1].m_bufp = cp;
	ip->i_flag |= IMOUNT;
nomount:
	brelse(bp);
}

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to 100 remembered
 * free blocks; the last of these is read to
 * obtain 100 more . . .
 *
 * no space on dev x/y -- when
 * the free list is exhausted.
 */
struct buf *
alloc(dev)
	int dev;
{
	int bno;
	register struct filsys *fp;
	register struct buf *bp;
	register int *ip;

	fp = getfs(dev);
	if(fp->s_nfree <= 0)
		goto nospace;
	bno = fp->s_free[--fp->s_nfree];
	if(bno == 0)
		goto nospace;
	if(fp->s_nfree <= 0) {
		bp = bread(dev, bno);
		ip = (int*) bp->b_addr;
		fp->s_nfree = *ip++;
		memcpy(fp->s_free, ip, 200);
		brelse(bp);
	}
	if(bno < fp->s_isize+2 || bno >= fp->s_fsize)
		panic("no space on dev");
	bp = getblk(dev, bno);
	memzero (bp->b_addr, 512);
	fp->s_fmod = 1;
	return(bp);

nospace:
	fp->s_nfree = 0;
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
void
free(dev, bno)
	int dev;
	int bno;
{
	register struct filsys *fp;
	register int *ip;
	register struct buf *bp;

	fp = getfs(dev);
	if(bno < fp->s_isize+2 || bno >= fp->s_fsize)
		panic("free block illegal");
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if(fp->s_nfree >= 100) {
		bp = getblk(dev, bno);
		ip = (int*) bp->b_addr;
		*ip++ = fp->s_nfree;
		memcpy(ip, fp->s_free, 200);
		fp->s_nfree = 0;
		bwrite(bp);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
}

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * 100 spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up 100 more.
 */
struct inode *
ialloc(dev)
	int dev;
{
	register struct filsys *fp;
	register int *bp;
	register struct inode *ip;
	int ino;

	fp = getfs(dev);
loop:
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino);
		if (ip==NULL)
			return(NULL);
		if(ip->i_mode == 0) {
			for(bp = &ip->i_mode; bp < &ip->i_addr[8];)
				*bp++ = 0;
			fp->s_fmod = 1;
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		goto loop;
	}
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * Free the specified I node
 * on the specified device.
 * The algorithm stores up
 * to 100 I nodes in the super
 * block and throws away any more.
 */
void
ifree(dev, ino)
	int dev;
	int ino;
{
	register struct filsys *fp;

	fp = getfs(dev);
	fp->s_inode[fp->s_ninode++] = ino;
	fp->s_fmod = 1;
}

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.
 * The algorithm is a linear
 * search through the mount table.
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
struct filsys *
getfs(dev)
	int dev;
{
	register struct mount *p;
	register struct buf *bp;

	for(p = &mount[0]; p < &mount[NMOUNT]; p++) {
		bp = p->m_bufp;
		if(bp && p->m_dev == dev)
			return (struct filsys*) bp->b_addr;
	}
	panic("no fs");
	return 0;
}

/*
 * update is the internal name of
 * 'sync'. It goes through the disk
 * queues to initiate sandbagged IO;
 * goes through the I nodes to write
 * modified nodes; and it goes through
 * the mount table to initiate modified
 * super blocks.
 */
void
update()
{
	register struct filsys *fp;
	register struct inode *ip;
	register struct mount *mp;
	register struct buf *bp;

	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			fp = (struct filsys*) mp->m_bufp->b_addr;
			if(fp->s_fmod == 0)
				continue;
			bp = getblk(mp->m_dev, 1);
			fp->s_time[0] = time[0];
			fp->s_time[1] = time[1];
			fp->s_fmod = 0;
			memcpy(bp->b_addr, fp, 512);
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		iupdat(ip);
	bflush(NODEV);
}

void
memzero (ptr, len)
	void *ptr;
	int len;
{
	register int *wp, bytes;

	wp = (int*) ptr;
	for (bytes=len; bytes>1; bytes-=2)
		*wp++ = 0;
	if (bytes > 0)
		*(char*) wp = 0;
}

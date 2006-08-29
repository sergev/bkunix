#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
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
iinit()
{
	register *cp, *bp;

	bp = bread(ROOTDEV, 1);
	cp = getblk(NODEV);
	if(u.u_error)
		panic();
	bcopy(bp->b_addr, cp->b_addr, 256);
	brelse(bp);
	mount[0].m_bufp = cp;
	mount[0].m_dev = ROOTDEV;
	cp = cp->b_addr;
	time[0] = cp->s_time[0];
	time[1] = cp->s_time[1];
}

/*
 * mount user file system
 */

minit()
{
	register *cp, *bp, *ip;

	bp = bread(MNTDEV, 1);
	if(bp->b_flags&B_ERROR)
		goto nomount;
	u.u_dirp = "/usr";
	ip = namei(0);
	mount[1].m_inodp = ip;
	mount[1].m_dev = MNTDEV;
	mount[1].m_bufp = cp = getblk(NODEV);
	bcopy(bp->b_addr, cp->b_addr, 256);
	ip->i_flag =| IMOUNT;
nomount:
	brelse(bp);
}

#ifdef	CONTIG

alloc(dev)
{
	register bno;
	register *fp, *bp;

	fp = getfs(dev);
	for(bno = fp->s_isize+2; bno < fp->s_fsize; bno++) {
		if(fp->s_bmap[bno>>3]&(1<<(bno&07))) {
			fp->s_bmap[bno>>3] =& ~(1<<(bno&07));
			bp = getblk(dev, bno);
			clrbuf(bp);
			fp->s_fmod = 1;
			return(bp);
		}
	}
	u.u_error = ENOSPC;
	return(0);
}

free(dev, bno)
{
	register *fp;

	fp = getfs(dev);
	fp->s_bmap[bno>>3] =| (1<<(bno&07));
}

#endif

#ifndef CONTIG

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
alloc(dev)
{
	int bno;
	register *bp, *ip, *fp;

	fp = getfs(dev);
	if(fp->s_nfree <= 0)
		goto nospace;
	bno = fp->s_free[--fp->s_nfree];
	if(bno == 0)
		goto nospace;
	if(fp->s_nfree <= 0) {
		bp = bread(dev, bno);
		ip = bp->b_addr;
		fp->s_nfree = *ip++;
		bcopy(ip, fp->s_free, 100);
		brelse(bp);
	}
	if(bno < fp->s_isize+2 || bno >= fp->s_fsize)
		panic();
	bp = getblk(dev, bno);
	clrbuf(bp);
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
free(dev, bno)
{
	register *fp, *bp, *ip;

	fp = getfs(dev);
	if(bno < fp->s_isize+2 || bno >= fp->s_fsize)
		panic();
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if(fp->s_nfree >= 100) {
		bp = getblk(dev, bno);
		ip = bp->b_addr;
		*ip++ = fp->s_nfree;
		bcopy(fp->s_free, ip, 100);
		fp->s_nfree = 0;
		bwrite(bp);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
}

#endif

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
ialloc(dev)
{
	register *fp, *bp, *ip;
	int i, j, k, ino;

	fp = getfs(dev);
#ifdef	CONTIG
	for(ino = 1; ino < fp->s_isize*16; ino++) {
		if((fp->s_imap[ino>>3]&(1<<(ino&07))) == 0)
			continue;
		ip = iget(dev, ino+1);
#endif
#ifndef	CONTIG
loop:
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino);
#endif
		if (ip==NULL)
			return(NULL);
		if(ip->i_mode == 0) {
#ifdef	CONTIG
			fp->s_imap[ino>>3] =& ~(1<<(ino&07));
#endif
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
#ifndef	CONTIG
		goto loop;
#endif
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

ifree(dev, ino)
{
	register *fp;

	fp = getfs(dev);
#ifdef	CONTIG
	fp->s_imap[(ino-1)>>3] =| (1<<((ino-1)&07));
#endif
#ifndef	CONTIG
	fp->s_inode[fp->s_ninode++] = ino;
#endif
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
getfs(dev)
{
	register struct mount *p;

	for(p = &mount[0]; p < &mount[NMOUNT]; p++)
	if(p->m_bufp != NULL && p->m_dev == dev) {
		p = p->m_bufp->b_addr;
		return(p);
	}
	panic();
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
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register *bp;

	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			ip = mp->m_bufp->b_addr;
			if(ip->s_fmod == 0)
				continue;
			bp = getblk(mp->m_dev, 1);
			ip->s_time[0] = time[0];
			ip->s_time[1] = time[1];
			ip->s_fmod = 0;
			bcopy(ip, bp->b_addr, 256);
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		iupdat(ip);
	bflush(NODEV);
}

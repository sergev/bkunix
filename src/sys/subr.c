#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "systm.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
bmap(ip, bn)
struct inode *ip;
int bn;
{
	register *bp, *bap, nb;
	int *nbp, d, i;

	d = ip->i_dev;
	if(bn & 0174000) {
		u.u_error = EFBIG;
		return(0);
	}
#ifdef	CONTIG
	if(ip->i_mode&ICONT) {
		if(bn >= ip->i_addr[1])
			return(0);
		else
			return(ip->i_addr[0]+bn);
	}
#endif
	if((ip->i_mode&ILARG) == 0) {

		/*
		 * small file algorithm
		 */

		if((bn & ~7) != 0) {

			/*
			 * convert small to large
			 */

			if ((bp = alloc(d)) == NULL)
				return(NULL);
			bap = bp->b_addr;
			for(i=0; i<8; i++) {
				*bap++ = ip->i_addr[i];
				ip->i_addr[i] = 0;
			}
			ip->i_addr[0] = bp->b_blkno;
			bdwrite(bp);
			ip->i_mode =| ILARG;
			goto large;
		}
		nb = ip->i_addr[bn];
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag =| IUPD;
		}
		return(nb);
	}

	/*
	 * large file algorithm
	 */

    large:
	i = bn>>8;
	if((nb=ip->i_addr[i]) == 0) {
		ip->i_flag =| IUPD;
		if ((bp = alloc(d)) == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
	} else
		bp = bread(d, nb);
	bap = bp->b_addr;

	/*
	 * normal indirect fetch
	 */

	i = bn & 0377;
	if((nb=bap[i]) == 0 && (nbp = alloc(d)) != NULL) {
		nb = nbp->b_blkno;
		bap[i] = nb;
		bdwrite(nbp);
		bdwrite(bp);
	} else
		brelse(bp);
	return(nb);
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user address space unless u_segflg is set.
 */
passc(c)
char c;
{

	if(subyte(u.u_base, c) < 0) {
		u.u_error = EFAULT;
		return(-1);
	}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(u.u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.  u_base is in the user's
 * address space unless u_segflg is set.
 */
cpass()
{
	register c;

	if(u.u_count == 0)
		return(-1);
	if((c=fubyte(u.u_base)) < 0) {
		u.u_error = EFAULT;
		return(-1);
	}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(c&0377);
}

/*
 * copy count words from from to to.
 */
bcopy(from, to, count)
int *from, *to;
{
	register *a, *b, c;

	a = from;
	b = to;
	c = count;
	do
		*b++ = *a++;
	while(--c);
}

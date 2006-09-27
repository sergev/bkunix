/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "systm.h"

char nofault;

/*
 * Check the address value received from user program.
 * Return 0 when the address is valid.
 */
int
bad_user_address(addr)
	register char *addr;
{
	/* The kernel wants to read into the struct user or from
	 * its own data sometimes. */
	if (! nofault)
		return 0;
	if ((unsigned) addr < TOPSYS || (unsigned) addr >= TOPUSR) {
		u.u_error = EFAULT;
		return 1;
	}
	return 0;
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
int
bmap(ip, bn)
	struct inode *ip;
	int bn;
{
	register struct buf *bp;
	register int *bap, nb;
	struct buf *nbp;
	int d, i;

	d = ip->i_dev;
	if(bn & 0174000) {
		u.u_error = EFBIG;
		return(0);
	}
	if((ip->i_mode&ILARG) == 0) {
		/*
		 * small file algorithm
		 */
		if((bn & ~7) != 0) {
			/*
			 * convert small to large
			 */
			bp = alloc(d);
			if (bp == NULL)
				return(NULL);
			bap = (int*) bp->b_addr;
			for(i=0; i<8; i++) {
				*bap++ = ip->i_addr[i];
				ip->i_addr[i] = 0;
			}
			ip->i_addr[0] = bp->b_blkno;
			bdwrite(bp);
			ip->i_mode |= ILARG;
			goto large;
		}
		nb = ip->i_addr[bn];
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag |= IUPD;
		}
		return(nb);
	}

	/*
	 * large file algorithm
	 */
large:
	i = bn>>8;
	nb = ip->i_addr[i];
	if(nb == 0) {
		ip->i_flag |= IUPD;
		bp = alloc(d);
		if (bp == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
	} else
		bp = bread(d, nb);
	bap = (int*) bp->b_addr;

	/*
	 * normal indirect fetch
	 */
	i = bn & 0377;
	nb = bap[i];
	if(nb == 0 && (nbp = alloc(d)) != NULL) {
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
int
passc(c)
	int c;
{
	if (bad_user_address (u.u_base))
		return -1;
	*u.u_base++ = c;
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	return(u.u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.  u_base is in the user's
 * address space unless u_segflg is set.
 */
int
cpass()
{
	register int c;

	if(u.u_count == 0)
		return(-1);
	if (bad_user_address (u.u_base))
		return -1;
	c = *u.u_base++;
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	return(c&0377);
}

void *
memcpy (pto, pfrom, nbytes)
	void *pto, *pfrom;
	unsigned int nbytes;
{
	register char *to, *from;
	register unsigned int bytes;

	if (nbytes == 0)
		return pto;
	to = (char*) pto;
	from = (char*) pfrom;
	bytes = nbytes;

	if ((int) to & 1) {
		/* In case of non-even destination - move one byte. */
		*to++ = *from++;
		--bytes;
	}
	if (! ((int) from & 1)) {
		/* Both pointers are even - move words. */
		while (bytes > 1) {
			*((int*)to) = *((int*)from);
			to += 2;
			from += 2;
			bytes -= 2;
		}
	}
	while (bytes-- > 0)
		*to++ = *from++;
	return pto;
}

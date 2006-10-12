/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "systm.h"

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
	if (u.u_segflg)
		return 0;
	if ((unsigned) addr < BOTUSR || (unsigned) addr >= TOPUSR) {
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
			for(nb=0; nb<8; nb++) {
				*bap++ = ip->i_addr[nb];
				ip->i_addr[nb] = 0;
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

void *
memcpy (pto, pfrom, nbytes)
	void *pto, *pfrom;
	unsigned int nbytes;
{
	register char *to, *from;
	register unsigned bytes;

	to = (char*) pto;
	from = (char*) pfrom;
	bytes = nbytes;
#if 0
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
#endif
	while (bytes-- > 0)
		*to++ = *from++;
	return pto;
}

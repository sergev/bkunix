/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "systm.h"

/*
 * Return the logical minimum
 * of the 2 arguments.
 */
static int
min(a, b)
	int a, b;
{
	if(a < b)
		return(a);
	return(b);
}

/*
 * Move 'an' bytes at byte location
 * &bp->b_addr[o] to/from (flag) the
 * user/kernel area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 *
 * There are 2 algorithms,
 * if source address, dest address and count
 * are all even in a user copy,
 * then the machine language copyin/copyout
 * is called.
 * If not, its done byte-by-byte with
 * cpass and passc.
 */
void
iomove(kdata, an, flag)
	char *kdata;
	int an, flag;
{
	register int n;

	n = an;
	if (flag==B_WRITE)
		memcpy (kdata, u.u_base, n);
	else
		memcpy (u.u_base, kdata, n);
	u.u_base += n;
	dpadd(u.u_offset, n);
	u.u_count -= n;
}

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 */
void
readi(aip)
	struct inode *aip;
{
	struct buf *bp;
	int lbn, bn, on;
	register int dn, n;
	register struct inode *ip;

	ip = aip;
	if(u.u_count == 0)
		return;
	if((ip->i_mode&IFMT) == IFCHR) {
		ttread();
		return;
	}

	do {
		lbn = bn = lshift(u.u_offset, -9);
		on = u.u_offset[1] & 0777;
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
			dn = dpcmp(ip->i_size0&0377, ip->i_size1,
				u.u_offset[0], u.u_offset[1]);
			if(dn <= 0)
				return;
			n = min(n, dn);
			if ((bn = bmap(ip, lbn)) == 0)
				return;
			dn = ip->i_dev;
		} else {
			dn = ip->i_addr[0];
		}
		bp = bread(dn, bn);
		iomove(bp->b_addr + on, n, B_READ);
		brelse(bp);
	} while(u.u_error==0 && u.u_count!=0);
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 */
void
writei(aip)
	struct inode *aip;
{
	struct buf *bp;
	int n, on;
	register int dn, bn;
	register struct inode *ip;

	ip = aip;
	if((ip->i_mode&IFMT) == IFCHR) {
		ttwrite();
		return;
	}
	if (u.u_count == 0)
		return;

	do {
		bn = lshift(u.u_offset, -9);
		on = u.u_offset[1] & 0777;
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
			if ((bn = bmap(ip, bn)) == 0)
				return;
			dn = ip->i_dev;
		} else
			dn = ip->i_addr[0];
		if(n == 512)
			bp = getblk(dn, bn); else
			bp = bread(dn, bn);
		iomove(bp->b_addr + on, n, B_WRITE);
		if(u.u_error != 0)
			brelse(bp); else
		if ((u.u_offset[1]&0777)==0)
			bwrite(bp); else
			bdwrite(bp);
		if(dpcmp(ip->i_size0&0377, ip->i_size1,
		  u.u_offset[0], u.u_offset[1]) < 0 &&
		  (ip->i_mode&(IFBLK&IFCHR)) == 0) {
			ip->i_size0 = u.u_offset[0];
			ip->i_size1 = u.u_offset[1];
		}
		ip->i_flag |= IUPD;
	} while(u.u_error==0 && u.u_count!=0);
}

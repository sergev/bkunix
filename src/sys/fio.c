#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "user.h"
#include "filsys.h"
#include "file.h"
#include "inode.h"
#include "reg.h"

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
getf(f)
{
	register *fp, rf;

	rf = f;
	if(rf<0 || rf>=NOFILE)
		goto bad;
	fp = u.u_ofile[rf];
	if(fp != NULL)
		return(fp);
bad:
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure and call closei
 * on last closef.
 */
closef(fp)
int *fp;
{
	register *rfp, *ip;

	rfp = fp;
	if(rfp->f_count <= 1)
		closei(rfp->f_inode);
	rfp->f_count--;
}

/*
 * Decrement reference count on an
 * inode due to the removal of a
 * referencing file structure.
 * On the last closei, switchout
 * to the close entry point of special
 * device handler.
 * Note that the handler gets called
 * on every open and only on the last
 * close.
 */
closei(ip)
int *ip;
{
	register *rip;

	rip = ip;
	if(rip->i_count <= 1 && (rip->i_mode&IFMT) == IFCHR)
		klclose();
	iput(rip);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * Called on all sorts of opens
 * and also on mount.
 */
openi(ip)
int *ip;
{
	register *rip;

	rip = ip;
	if((rip->i_mode&IFMT) == IFCHR)
		klopen();
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions except for EXEC where
 * at least one of the EXEC bits must
 * be on.
 */
access(aip, mode)
int *aip;
{
	register *ip, m;

	ip = aip;
	m = mode;
	if(m == IEXEC && (ip->i_mode & 
		(IEXEC | (IEXEC>>3) | (IEXEC>>6))) == 0)
			goto bad;
	return(0);
bad:
	u.u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
owner()
{
	register struct inode *ip;

	if ((ip = namei(0)) == NULL)
		return(NULL);
	return(ip);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc()
{
	register i;

	for (i=0; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
falloc()
{
	register struct file *fp;
	register i;

	if ((i = ufalloc()) < 0)
		return(NULL);
	for (fp = &file[0]; fp < &file[NFILE]; fp++)
		if (fp->f_count==0) {
			u.u_ofile[i] = fp;
			fp->f_count++;
			fp->f_offset[0] = 0;
			fp->f_offset[1] = 0;
			return(fp);
		}
	u.u_error = ENFILE;
	return(NULL);
}

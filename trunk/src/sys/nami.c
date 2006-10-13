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
#include "systm.h"
#include "buf.h"

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
int
uchar()
{
	if (bad_user_address (u.u_dirp))
		return -1;
	return *u.u_dirp++;
}

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * flag = 0 if name is saught
 *	1 if name is to be created
 *	2 if name is to be deleted
 */
struct inode *
namei(flag)
	int flag;
{
	register struct inode *dp;
	register int c;
	register char *cp;
	int eo;
	struct buf *bp;

	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */
	dp = u.u_cdir;
	c = uchar();
	if(c == '/')
		dp = rootdir;
	iget(dp->i_dev, dp->i_number);
	while(c == '/')
		c = uchar();
	if(c == '\0' && flag != 0) {
		u.u_error = ENOENT;
		goto out;
	}
cloop:
	/*
	 * Here dp contains pointer
	 * to last component matched.
	 */
	if(u.u_error)
		goto out;
	if(c == '\0')
		return(dp);

	/*
	 * If there is another component,
	 * dp must be a directory and
	 * must have x permission.
	 */
	if((dp->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto out;
	}
	if(access(dp, IEXEC))
		goto out;

	/*
	 * Gather up name into
	 * users' dir buffer.
	 */
	cp = &u.u_dbuf[0];
	while(c!='/' && c!='\0' && u.u_error==0) {
		if(cp < &u.u_dbuf[DIRSIZ])
			*cp++ = c;
		c = uchar();
	}
	while(cp < &u.u_dbuf[DIRSIZ])
		*cp++ = '\0';
	while(c == '/')
		c = uchar();
	if(u.u_error)
		goto out;
#ifdef MNTOPTION
	/*
	 * Special handling for ".." allowing chdir out of mounted
	 * file system. We know NMOUNT == 2.
	 */
	if (u.u_dbuf[0]=='.' && u.u_dbuf[1]=='.' && u.u_dbuf[2]=='\0' &&
	    dp->i_number == ROOTINO && dp->i_dev == mount[1].m_dev &&
	    mount[1].m_inodp != 0) {
		iput(dp);
		dp = mount[1].m_inodp;
		dp->i_count++;
	}
#endif

	/*
	 * Set up to search a directory.
	 */
	u.u_offset = 0;
	eo = 0;
	u.u_count = dp->i_size >> 4;
	bp = NULL;
eloop:
	/*
	 * If at the end of the directory,
	 * the search failed. Report what
	 * is appropriate as per flag.
	 */
	if(u.u_count == 0) {
		if(bp != NULL)
			brelse(bp);
		if(flag==1 && c=='\0') {
			if(access(dp, IWRITE))
				goto out;
			u.u_pdir = dp;
			if(eo)
				u.u_offset = eo-DIRSIZ-2;
			else
				dp->i_flag |= IUPD;
			return(NULL);
		}
		u.u_error = ENOENT;
		goto out;
	}

	/*
	 * If offset is on a block boundary,
	 * read the next directory block.
	 * Release previous if it exists.
	 */
	if(((int) u.u_offset & 0777) == 0) {
		if(bp != NULL)
			brelse(bp);
		bp = bread(dp->i_dev,
			bmap(dp, (int) (u.u_offset >> 9)));
	}

	/*
	 * Note first empty directory slot
	 * in eo for possible creat.
	 * String compare the directory entry
	 * and the current component.
	 * If they do not match, go back to eloop.
	 */
	memcpy(&u.u_dent, bp->b_addr + ((int) u.u_offset & 0777), DIRSIZ+2);
	u.u_offset += DIRSIZ+2;
	u.u_count--;
	if(u.u_dent.u_ino == 0) {
		if(eo == 0)
			eo = u.u_offset;
		goto eloop;
	}
	for(cp = &u.u_dbuf[0]; cp < &u.u_dbuf[DIRSIZ]; cp++)
		if(*cp != cp[u.u_dent.u_name - u.u_dbuf])
			goto eloop;

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */
	if(bp != NULL)
		brelse(bp);
	if(flag==2 && c=='\0') {
		if(access(dp, IWRITE))
			goto out;
		return(dp);
	}
	iput(dp);
	dp = iget(dp->i_dev, u.u_dent.u_ino);
	if(dp == NULL)
		return(NULL);
	goto cloop;
out:
	iput(dp);
	return(NULL);
}

/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "systm.h"
#include "user.h"
#include "reg.h"
#include "file.h"
#include "inode.h"

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
static void
rdwr(mode)
	int mode;
{
	register struct file *fp;
	register int m;

	m = mode;
	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if((fp->f_flag&m) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (char*) u.u_arg[0];
	u.u_count = u.u_arg[1];
	u.u_offset[1] = fp->f_offset[1];
	u.u_offset[0] = fp->f_offset[0];
	if(m==FREAD)
		readi(fp->f_inode);
	else
		writei(fp->f_inode);
	dpadd(fp->f_offset, u.u_arg[1]-u.u_count);
	u.u_ar0[R0] = u.u_arg[1]-u.u_count;
}

/*
 * read system call
 */
void
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
void
write()
{
	rdwr(FWRITE);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
static void
open1(ip, mode, trf)
	struct inode *ip;
	int mode;
	int trf;
{
	register struct file *fp;
	register struct inode *rip;
	register int m;

	rip = ip;
	m = mode;
	if(trf != 2) {
		if(m&FREAD)
			access(rip, IREAD);
		if(m&FWRITE) {
			access(rip, IWRITE);
			if((rip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf)
		itrunc(rip);
	fp = falloc();
	if(fp == NULL)
		goto out;
	fp->f_flag = m&(FREAD|FWRITE);
	fp->f_inode = rip;
	m = u.u_ar0[R0];
	openi(rip);
	if(mode <= 3 && u.u_error == 0)
		return;
	u.u_ofile[m] = NULL;
	if(mode > 3) {
		fp->f_count--;
		return;
	}
out:
	iput(rip);
}

/*
 * open system call
 */
void
open()
{
	register struct inode *ip;

	ip = namei(0);
	if(ip == NULL)
		return;
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0);
}

/*
 * creat system call
 */
void
creat()
{
	register struct inode *ip;

	ip = namei(1);
	if(ip == NULL) {
		if(u.u_error)
			return;
		ip = maknode(u.u_arg[1]&07777);
		if (ip==NULL)
			return;
		open1(ip, FWRITE, 2);
	} else
		open1(ip, FWRITE, 1);
}

/*
 * close system call
 */
void
close()
{
	register struct file *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_ar0[R0]] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
void
seek()
{
	int n[2];
	register struct file *fp;
	register int t;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	t = u.u_arg[1];
	if(t > 2) {
		n[1] = u.u_arg[0]<<9;
		n[0] = u.u_arg[0]>>7;
		if(t == 3)
			n[0] &= 0777;
	} else {
		n[1] = u.u_arg[0];
		n[0] = 0;
		if(t!=0 && n[1]<0)
			n[0] = -1;
	}
	switch(t) {

	case 1:
	case 4:
		n[0] += fp->f_offset[0];
		dpadd(n, fp->f_offset[1]);
		break;

	default:
		n[0] += fp->f_inode->i_size0&0377;
		dpadd(n, fp->f_inode->i_size1);

	case 0:
	case 3:
		;
	}
	fp->f_offset[1] = n[1];
	fp->f_offset[0] = n[0];
}

/*
 * link system call
 */
void
link()
{
	register struct inode *ip, *xp;

	ip = namei(0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	u.u_dirp = (char*) u.u_arg[1];
	xp = namei(1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag |= IUPD;

out:
	iput(ip);
}

/*
 * mknod system call
 */
void
mknod()
{
	register struct inode *ip;

	ip = namei(1);
	if(ip != NULL) {
		u.u_error = EEXIST;
		goto out;
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}

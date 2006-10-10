/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef FILE_H
#define FILE_H 1

/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	char		f_flag;
	char		f_count;	/* reference count */
	struct inode	*f_inode;	/* pointer to inode structure */
	unsigned int	f_offset[2];	/* read/write character pointer */
};
extern struct file file[NFILE];

/* flags */
#define	FREAD	01
#define	FWRITE	02

#ifdef KERNEL
struct file *getf();
struct file *falloc();
int ufalloc();
void closef();
void klopen();
void klclose();
void klsgtty();
#endif

#endif /* FILE_H */

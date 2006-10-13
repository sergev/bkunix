/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef INODE_H
#define INODE_H 1

/*
 * The I node is the focus of all
 * file activity in unix. There is a unique
 * inode allocated for each active file,
 * each current directory, each mounted-on
 * file, text file, and the root. An inode is 'named'
 * by its dev/inumber pair. (iget/iget.c)
 * Data, from mode on, is read in
 * from permanent inode on volume.
 */
struct	inode
{
	char	i_flag;
	char	i_count;	/* reference count */
	int	i_dev;		/* device where inode resides */
	int	i_number;	/* i number, 1-to-1 with device address */
	int	i_mode;
	char	i_nlink;	/* directory entries */
	char	i_uid;		/* owner */
	unsigned int i_size0;	/* most significant of size */
	unsigned int i_size1;	/* least sig */
	int	i_addr[8];	/* device addresses constituting file */
};
extern struct inode inode[NINODE];

/* flags */
#define	IUPD	02		/* inode has been modified */
#define	IACC	04		/* inode access time to be updated */
#define	IMOUNT	010		/* inode is mounted on */

/* modes */
#define	IALLOC	0100000		/* file is used */
#define	IFMT	060000		/* type of file */
#define		IFDIR	040000	/* directory */
#define		IFCHR	020000	/* character special */
#define		IFBLK	060000	/* block special, 0 is regular */
#define	ILARG	010000		/* large addressing algorithm */
#define	ICONT	01000		/* contiguous file */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100

#ifdef KERNEL
struct inode *ialloc();
struct inode *namei();
struct inode *iget();
struct inode *maknode();
void ifree();
void openi();
void closei();
void iput();
void iupdat();
void itrunc();
void readi();
void writei();
void wdir();
int bmap();
int access();
#endif

#endif /* INODE_H */

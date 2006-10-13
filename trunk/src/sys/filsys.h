/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef FILSYS_H
#define FILSYS_H 1

/*
 * Definition of the unix super block.
 * The root super block is allocated and
 * read in iinit/alloc.c. Subsequently
 * a super block is allocated and read
 * with the initial mount (minit/alloc.c)
 * A disk block is ripped off for storage.
 * See alloc.c for general alloc/free
 * routines for free list and I list.
 */
struct	filsys
{
	int	s_isize;	/* size in blocks of I list */
	int	s_fsize;	/* size in blocks of entire volume */
	int	s_nfree;	/* number of in core free blocks (0-100) */
	int	s_free[100];	/* in core free blocks */
	int	s_ninode;	/* number of in core I nodes (0-100) */
	int	s_inode[100];	/* in core free I nodes */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during I list manipulation */
	char	s_fmod;		/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	long	s_time;		/* current date of last update */
	int	pad[50];
};

#ifdef KERNEL
struct filsys *getfs();
#endif

#endif /* FILSYS_H */

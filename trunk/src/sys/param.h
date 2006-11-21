/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef PARAM_H
#define PARAM_H 1

/*
 * uncomment if compiled for a machine with an EIS emulator
 * in the FDD controller ROM
 */
/*#define EIS 1*/

/*
 * remove the comments to enable background process,
 * mount user file system on /usr and enable clock, respectively
 */
/*#define BGOPTION	1*/
#define CLOCKOPT	1
#define MNTOPTION	1
/*#define COREOPT	1*/

/*
 * the following 4 variables may be modified.
 */
#define NPROC	3		/* max number of processes */
#define BOTSYS	0120000		/* must not be an expression */
#define TOPSYS	0160000		/* must not be an expression */
#define SYSSIZ	((TOPSYS-BOTSYS)/1024) 	/* system size in 1K bytes */
#define NBLKS	1600		/* 2-sided, 80 tracks, 9 sectors */
#define USRSIZ	27		/* size for extended memory */
#define BOTUSR	02000		/* must not be an expression */
#define UCORE	(USRSIZ*1024)	/* bytes */
#define TOPUSR	u.u_top
#define SMALL	(15*1024)
#define LARGE	(27*1024)
#define SWPSIZ	64		/* was (USRSIZ*2+1) */
#ifdef BGOPTION
#define NSWAP	(NPROC*SWPSIZ+2)
#endif
#ifndef BGOPTION
#define NSWAP	((NPROC-1)*SWPSIZ) /* was ((NPROC-1)*SWPSIZ+1) */
#endif
#define SWPLO	(NBLKS-NSWAP)

#define NBUF	6 		/* size of buffer cache */
#define NINODE	20		/* number of in core inodes */
#define NFILE	20		/* number of in core file structures */
#define NMOUNT	2		/* number of mountable file systems */
#ifndef LOWSTACK
#define SSIZE	1280		/* initial stack size (in bytes) */
#else
#define SSIZE	0		/* stack included in .bss */
#endif
#define NOFILE	15		/* max open files per process */
#define CANBSIZ	132		/* max size of typewriter line */
#define NCLIST	25		/* max total clist size */

/*
 * priorities
 * probably should not be
 * altered too much
 */
#ifdef BGOPTION
#define PSWP	-50
#endif
#ifndef BGOPTION
#define PSWP	-100
#endif
#define PRIBIO	-50
#define PSLEP	90

/*
 * signals
 * dont change
 */
#define NSIG	20
#define		SIGHUP	1	/* hangup */
#define		SIGINT	2	/* interrupt (rubout) */
#define		SIGQIT	3	/* quit (FS) */
#define		SIGINS	4	/* illegal instruction */
#define		SIGTRC	5	/* trace or breakpoint */
#define		SIGIOT	6	/* iot */
#define		SIGEMT	7	/* emt */
#define		SIGFPT	8	/* floating exception */
#define		SIGKIL	9	/* kill */
#define		SIGBUS	10	/* bus error */
#define		SIGSEG	11	/* segmentation violation */
#define		SIGSYS	12	/* sys */
#define		SIGPIPE	13	/* end of pipe */
#define		SIGCLK	14	/* alarm clock */

/*
 * fundamental constants
 * cannot be changed
 */
#define USIZE	512		/* size of user block (bytes) */
#define NULL	0
#define NODEV	(-1)
#define ROOTINO	1		/* i number of all roots */
#define DIRSIZ	14		/* max characters per directory */

/*
 * configuration dependent variables
 */
#define ROOTDEV bootdev
#define SWAPDEV bootdev

#endif /* PARAM_H */

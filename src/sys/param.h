/* comment out EIS if no EIS chip on machine */
#define EIS		1

/*
 * remove the comments to enable background process,
 * mount user file system on /usr and enable clock, respectively
 */
/*#define BGOPTION	1*/
#define MOUNT_USR	0
#define CLOCKOPT	1

/*
 * the following 4 variables may be modified.
 */
#define	NPROC	3	/* max number of processes */
#define	NBLKS	500	/* 256-word blocks per diskette */
#define	SYSSIZ	8 	/* system size in 1K words */
#define	USRSIZ	12	/* user program size in 1K words */
#define UCORE	(USRSIZ*32)
#define TOPSYS	(SYSSIZ*2048)
#define	TOPUSR	((SYSSIZ+USRSIZ)*(unsigned)2048)
#define SWPSIZ	(USRSIZ*4+1)
#ifdef BGOPTION
#define NSWAP	(NPROC*SWPSIZ+2)
#endif
#ifndef BGOPTION
#define NSWAP	((NPROC-1)*SWPSIZ+1)
#endif
#define	SWPLO	(NBLKS-NSWAP)

#define	NBUF	6 		/* size of buffer cache */
#define	NINODE	20		/* number of in core inodes */
#define	NFILE	20		/* number of in core file structures */
#define	NMOUNT	2		/* number of mountable file systems */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	132		/* max size of typewriter line */
#define	NCLIST	25		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */

/*
 * priorities
 * probably should not be
 * altered too much
 */
#ifdef BGOPTION
#define	PSWP	-50
#endif
#ifndef BGOPTION
#define PSWP	-100
#endif
#define	PRIBIO	-50
#define	PSLEP	90

/*
 * signals
 * dont change
 */
#define	NSIG	20
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
#define	USIZE	8		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570

/*
 * Comment out the definition of CLOCK for LSI-11;
 * otherwise, set CLOCK to be 0177546 or 0172540 for the line
 * frequency or programmable clock, respectively.
 * Then recompile main.c and clock.c
 */
/*
#define CLOCK	0177546
*/

/*
 * configuration dependent variables
 */
#define ROOTDEV 0
#define SWAPDEV 1
#define MNTDEV  1

void *memcpy();
void *memset();

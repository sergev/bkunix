/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "user.h"
#include "systm.h"
#include "proc.h"
#include "inode.h"

char		canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
struct inode	*rootdir;		/* pointer to inode of root directory */
int		lbolt;			/* time of day in 60th not in time */
int		time[2];		/* time in sec from 1970 */
int		tout[2];		/* time of day of next sleep */

struct mount	mount[NMOUNT];
int		cpid;			/* current process ID */

#ifdef BGOPTION
struct proc	proc[NPROC+2];
#else
struct proc	proc[NPROC];
#endif

/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[] = {
	0104413,	/* sys exec; init; initp */
	TOPSYS+014,
	TOPSYS+010,
	0000777,	/* br . */
	TOPSYS+014,	/* initp: init; 0 */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};

#define TP_STATUS	(*(volatile unsigned char*) 0177564)
#define TP_BYTE		(*(volatile unsigned char*) 0177566)

void
debug_putc (int c)
{
again:
	while (! (TP_STATUS & 0x80))
		continue;
	TP_BYTE = c;

	if (c == '\n') {
		c = '\r';
		goto again;
	}
}

void
debug_puts (char *s)
{
	while (*s)
		debug_putc (*s++);
}

/*
 * Initialization code.
 * Called from mch.s as
 * soon as a stack
 * has been established.
 * Functions:
 *	clear and free user core
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *
 * loop at loc 40006 in user mode -- /etc/init
 *	cannot be executed.
 */
void
unixmain()
{
	/*
	 * set up system process
	 */
	proc[0].p_stat = SRUN;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */
#ifdef CLOCK
	*(int*) CLOCK = 0115;
#endif
	cinit();
	binit();
	iinit();
	rootdir = iget(ROOTDEV, ROOTINO);
	u.u_cdir = iget(ROOTDEV, ROOTINO);
#ifdef	MOUNT_USR
	minit();	/* mount user file system */
#endif

	/*
	 * make init process
	 * with system process
	 */
debug_puts ("before memcpy\n");
	memcpy(TOPSYS, icode, sizeof icode);
debug_puts ("after memcpy\n");
	/*
	 * Return goes to loc. 0 of user init
	 * code just copied out.
	 */
}

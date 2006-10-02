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
	BOTUSR+014,
	BOTUSR+010,
	0000777,	/* br . */
	BOTUSR+014,	/* initp: init; 0 */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};

int
ttputc (c)
        int c;
{
#ifdef BK
asm("mov 4(r5),r0");
asm("mov r5,-(sp)");
asm("jsr pc,*$0102234");
asm("mov (sp)+,r5");
#else
asm("clr *$tps");
asm("mov 4(r5),r0");
asm("jsr pc, putc");
asm("jbr 9f");
asm("tps = 0177564");
asm("tpb = 0177566");
asm("putc: ");
asm("tstb    tps");
asm("jge     putc");
asm("cmp     r0,$012");
asm("jne     1f");
asm("mov     $015,r0");
asm("jsr     pc,putc");
asm("mov     $012,r0");
asm("1:");
asm("mov     r0,*$tpb");
asm("rts     pc");
asm("9: ");
#endif
}

void
ttputs (s)
	char *s;
{
	while(ttputc(*s++));
}
/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then halts.
 */
void panichalt(s)
	char *s;
{
	ttputs("panic: ");
	ttputs(s);
	ttputs("\r\n");
#ifdef BK
	for(;;);
#else
	for (;;) 
		asm("0");
#endif
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
	memcpy(BOTUSR, icode, sizeof icode);

	/*
	 * Return goes to loc. 0 of user init
	 * code just copied out.
	 */
#ifdef DEBUG
	debug_printf ("unixmain done\n");
#endif
}

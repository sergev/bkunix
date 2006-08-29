#
/*
 *	Copyright 1975 Bell Telephone Laboratories Inc
 */

#include "param.h"
#include "user.h"
#include "systm.h"
#include "proc.h"
#include "inode.h"

/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[]
{
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

main()
{
	extern schar;
	register i1, *p;

	/*
	 * zero and free all of core
	 */


	/*
	 * determine clock
	 */


	/*
	 * set up system process
	 */

	proc[0].p_stat = SRUN;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */

#ifdef CLOCK
	CLOCK->integ = 0115;
#endif
	cinit();
	binit();
	iinit();
	rootdir = iget(ROOTDEV, ROOTINO);
	u.u_cdir = iget(ROOTDEV, ROOTINO);
#ifdef	MOUNT
	minit();	/* mount user file system */
#endif

	/*
	 * make init process
	 * with system process
	 */

	copyout(icode, TOPSYS, sizeof icode);
	/*
	 * Return goes to loc. 0 of user init
	 * code just copied out.
	 */
}

/*
 * FD/HDD firmware ROM interface for BK UNIX
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "buf.h"
#include "user.h"

struct fdio {
	int		fd_csrw;	/* Copy of fdc state register */
	int		fd_curtrk;	/* Current track */
	unsigned char	fd_trktab [4];	/* Table of current tracks */
	int		fd_tdown;	/* Head down time (in SOB loops) */
	int		fd_tstep;	/* Track change time */
	unsigned char	fd_trkcor;	/* Write precompensation track number */
	unsigned char	fd_bretry;	/* Max retries */
	unsigned char	fd_flags;	/* Driver data */
	unsigned char	fd_fillb;	/* Fill byte for formatting */
	int		fd_flgptr;	/* Pointer to flag byte */
	unsigned char	fd_flgtab [4];	/* Flag table */
	int		fd_addr;	/* Buffer address in RAM */
	int		fd_wcnt;	/* Number of words to transfer */
	unsigned char	fd_side;	/* Disk side */
	unsigned char	fd_trk;		/* Track */
	unsigned char	fd_unit;	/* Disk unit number */
	unsigned char	fd_sector;	/* Sector */
	unsigned char	fd_wrk1 [18];	/* Driver working area */
	int		fd_maxsec;	/* Number of sectors per track */
	unsigned char	fd_wrk2 [4];	/* Driver working area */
};

struct devtab fdtab;
#define NFDBLK 1600

static struct fdio ioarea;

int stopdelay;
#define STOPDELAY 5

void fdstart()
{
	register struct buf *bp;
	register struct fdio *r3;
	register char *r2;
	struct buf *savbp;

	if ((bp = fdtab.d_actf) == 0)
		return;

	/* fdtab.d_active++ ; */
	fdtab.d_actf = (struct buf *) bp->b_link; /* unlink immediately */
	bp->b_flags |= B_DONE;
	savbp = bp;
	stopdelay = STOPDELAY;

	ioarea.fd_unit = bp->b_dev;
	r3 = &ioarea;
	r2 = bp->b_addr;
	asm("mov 4(r4), r1");	/* word cnt */
	if (bp->b_flags & B_READ)
		asm("neg r1");;	/* negative cnt == write */
	asm("mov 010(r4), r0"); /* blk num */
	asm("mov r5,-(sp)");	/* r5 will be corrupted */
	asm("jsr pc, *$0160004");
	asm("bcs 1f");
	asm("mov (sp)+,r5");
	asm("jmp cret");

	asm("1: mov (sp)+,r5");
	bp = savbp;
	bp->b_flags |= B_ERROR;
}

void fdstrategy( abp )
	struct buf *abp;
{
	register struct buf *bp;

	bp = abp;

	if( bp->b_blkno >= NFDBLK ) {
		bp->b_flags |= B_DONE | B_ERROR;
		return;
	}

	bp->b_link = 0;
	spl7();
	if( fdtab.d_actf == 0 )
		fdtab.d_actf = bp;
	else
		fdtab.d_actl->b_link = (int *) bp;

	fdtab.d_actl = bp;
	if( fdtab.d_active == 0 )
		fdstart();
	spl0();
}

void fdinit()
{
	/* Using BIOS i/o area at address 02000. */
	memcpy (&ioarea, (char*) 02000, sizeof(ioarea));

	/* Disable write precompensation. */
	ioarea.fd_trkcor = 999;
}

void fdstop()
{
	/* Stop floppy motor. */
	*(int*) 0177130 = 0;
	stopdelay = 0;
}

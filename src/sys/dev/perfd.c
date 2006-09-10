/*
 * FD disk driver
 *
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "buf.h"
#include "user.h"

#define INITSEEK 010000
#define IENABLE	020000
#define NOERROR	040000
#define FDREAD	0
#define FDWRITE	0100000

#define POWER	02000
#define FORMAT	04000
#define BUSTIME	010000
#define PARERR	020000
#define HDERR	040000
#define SDONE	0100000

#define FDVECT	0124
#define FDPS	0126

#define NFDBLK	616
#define FDADR0 0177600
#define FDADR1	0177610

struct {
	int integ;
};

struct {
	int fdcont;
	int fdstat;
	int fdba;
	int fdclock;
};

struct devtab fdtab;

int hbuf[40];
int *hp hbuf;

fdstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if (bp->b_blkno >= NFDBLK) {
		bp->b_flags |= B_DONE | B_ERROR;
		return;
	}
	bp->b_link = 0;
	spl7();
	if (fdtab.d_actf==0)
		fdtab.d_actf = bp;
	else
		fdtab.d_actl->b_link = bp;
	fdtab.d_actl = bp;
	if (fdtab.d_active==0)
		fdstart();
	spl0();
}

fdstart()
{
	register struct buf *bp;
	register int *FD;
	register com;

	if ((bp = fdtab.d_actf) == 0)
		return;
	fdtab.d_active++;
	if(bp->b_dev == 0)
		FD = FDADR0;
	else
		FD = FDADR1;
	while((FD->fdstat&SDONE) == 0);
	FD->fdba = bp->b_addr;
	com = bp->b_blkno;
	if(bp->b_flags&B_READ)
		com |= FDREAD;
	else
		com |= FDWRITE;
	FD->fdcont = IENABLE|com;
}

fdintr()
{
	register struct buf *bp;
	register int *FD;

	if (fdtab.d_active == 0)
		return;
	bp = fdtab.d_actf;
	fdtab.d_active = 0;
	if(bp->b_dev == 0)
		FD = FDADR0;
	else
		FD = FDADR1;
	if(FD->fdstat&(HDERR|PARERR|BUSTIME)) {
		*hp++ = FD->fdstat;
		if(hp >= &hbuf[40])
			hp = hbuf;
		if (++fdtab.d_errcnt <= 10) {
			fdstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	fdtab.d_errcnt = 0;
	if((bp->b_wcount += 256) == 0) {
		fdtab.d_actf = bp->b_link;
		bp->b_flags |= B_DONE;
#ifdef BGOPTION
		wakeup(bp);
#endif
	} else {
		bp->b_addr += 512;
		bp->b_blkno++;
	}
	fdstart();
}

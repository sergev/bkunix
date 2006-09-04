/*
 * FD disk driver
 *
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "buf.h"
#include "user.h"

#define ERR	0100000
#define	INIT	040000
#define TR	0200
#define	IENABLE	0100
#define DONE	040
#define	UNIT	020
#define	FILLBUF	0
#define	EMPBUF	02
#define FDWRITE	04
#define FDREAD	06
#define	GO	01

#define FDVECT	0264
#define FDPS	0266

#define NFD	2
#define NFDBLK	500
#define	FDADR	0177170

/*
 * Comment out next line if logical to physical track mapping
 * is not to be performed.  Also, change boot.s  and header.s
 */
#define IBMS	1

struct fdregs {
	int	rxcs;
	int	rxdb;
};

struct devtab fdtab;

int sect;

/* debugging circular buffer
int hbuf[40];
int *hp hbuf;
*/

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
		fdtab.d_actl->b_link = (int*) bp;
	fdtab.d_actl = bp;
	if (fdtab.d_active==0) {
		fdstart();
	}
	spl0();
}

fdstart()
{
	register struct buf *bp;
	register struct fdregs *FD;
	register char *cp;

	if ((bp = fdtab.d_actf) == 0)
		return;
	fdtab.d_active++;
	FD = (struct fdregs*) FDADR;
	if((bp->b_flags&B_READ) == 0) {
		cp = bp->b_addr + (sect << 7);
		FD->rxcs = FILLBUF | GO;
		while((FD->rxcs&DONE) == 0) {
			if(FD->rxcs&TR)
				FD->rxdb = *cp++;
		}
		if(FD->rxcs & ERR) {
			panic();
		}
	}
	while((FD->rxcs&DONE) == 0);
	FD->rxcs = (bp->b_dev<<4) | IENABLE | ((bp->b_flags&B_READ)?FDREAD:FDWRITE) | GO;
	while((FD->rxcs&TR) == 0);

	FD->rxdb = ((((int) bp->b_blkno * 4 + sect) * 3) % 26) + 1;
	while((FD->rxcs&TR) == 0);
#ifdef IBMS
	{
	int trk = ((int) bp->b_blkno * 4 + sect) / 26 + 1;
	if (trk == 77)
		trk = 0;
	FD->rxdb = trk;
	}
#else
	FD->rxdb = ((int) bp->b_blkno * 4 + sect) / 26;
#endif
}

fdintr()
{
	register struct buf *bp;
	register struct fdregs *FD;
	register char *cp;

	if (fdtab.d_active == 0)
		return;
	bp = fdtab.d_actf;
	fdtab.d_active = 0;
	FD = (struct fdregs*) FDADR;
	if(FD->rxcs&ERR) {
/*
		*hp++ = FD->rxdb;
		if(hp >= &hbuf[40])
			hp = hbuf;
*/
		if (++fdtab.d_errcnt <= 10) {
			fdstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	fdtab.d_errcnt = 0;
	if(bp->b_flags&B_READ) {
		cp = bp->b_addr + (sect << 7);
		FD->rxcs = EMPBUF | GO;
		while((FD->rxcs&DONE) == 0) {
			if(FD->rxcs&TR)
				*cp++ = FD->rxdb;
		}
		if(FD->rxcs & ERR) {
			panic();
		}
	}
	if((bp->b_wcount += 64) == 0) {
		fdtab.d_actf = (struct buf*) bp->b_link;
		bp->b_flags |= B_DONE;
#ifdef BGOPTION
		wakeup(bp);
#endif
		sect = 0;
	} else {
		if(++sect == 4) {
			sect = 0;
			bp->b_blkno++;
			bp->b_addr += 512;
		}
	}
	fdstart();
}

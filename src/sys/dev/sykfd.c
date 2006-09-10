/*
 * Sykes FD disk driver
 *
 * Copyright 1976 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "buf.h"
#include "user.h"

#define RESET	0200
#define UNIT1	0201
#define UNIT2	0202
#define TERM	0203
#define	IENABLE	0100
#define FDONE	0200
#define FDWRITE	040
#define FDREAD	0
#define BUSY	04

#define FAULT	0100
#define CRCERR	010

#define FDVECT	0174
#define FDPS	0176

#define NFD	2
#define NFDBLK	500
#define	FDADR	0176000

struct {
	int integ;
};

struct {
	int	flfg;
	int	flcm;
	int	flst;
	int	flda;
};

struct devtab fdtab;

int	sect;

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
	register int i;
	register char *cp;

	if ((bp = fdtab.d_actf) == 0)
		return;
	fdtab.d_active++;
	FDADR->flcm = RESET;
	FDADR->flcm = bp->b_dev?UNIT2:UNIT1;
	FDADR->flcm = (i = bp->b_blkno*4)/26;
	FDADR->flcm = ((i%26) + 1) | ((bp->b_flags&B_READ)?FDREAD:FDWRITE);
	if((bp->b_flags&B_READ) == 0) {
		cp = bp->b_addr+sect*128;
		for(i = 0; i < 128; i++) {
			while((FDADR->flfg&FDONE) == 0);
			FDADR->flda = *cp++;
		}
	}
}

fdintr()
{
	register struct buf *bp;
	register int i;
	register char *cp;

	if (fdtab.d_active == 0)
		return;
	bp = fdtab.d_actf;
	fdtab.d_active = 0;
	FDADR->flfg = 0;
	if(bp->b_flags&B_READ) {
		cp = bp->b_addr+sect*128;
		for(i = 0; i < 128; i++) {
			while((FDADR->flfg&FDONE) == 0);
			*cp++ = FDADR->flda;
		}
	}
	FDADR->flcm = TERM;
	while(FDADR->flst&BUSY);
	if(FDADR->flst&(FAULT|CRCERR)) {
		*hp++ = FDADR->flst;
		if(hp >= &hbuf[40])
			hp = hbuf;
		if (++fdtab.d_errcnt <= 10) {
			fdstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	fdtab.d_errcnt = 0;
	if((bp->b_wcount += 64) == 0) {
		fdtab.d_actf = bp->b_link;
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

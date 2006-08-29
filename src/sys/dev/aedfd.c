#
/*
 *	Copyright 1976 Bell Telephone Laboratories Inc
 */

/*
 * AED 6200lp disk driver
 */

#include "param.h"
#include "buf.h"
#include "user.h"


#define	FDVECT	0170
#define	FDPS	0172
#define	FDADDR	0164000
#define	NFDBLK	1232
#define	NFD	2


#define	CMD0	0
#define	CMD1	01000
#define	CMD2	02000
#define	CMD3	03000
#define	WSC0	0
#define	RS	020000
#define	WSC1	040000
#define	INSS	060000
#define	WSC2	0100000
#define	RSID	0120000
#define	WSC3	0140000
#define	RZSK	0160000

#define	MO	0400
#define	IS	0200
#define	UN0	0
#define	UN1	04000
#define	UN2	010000
#define	UN3	014000
#define	ERR	0100000
#define	DONE	0200
#define	IENB	0100
#define	IF	0100000
#define	IX	040000
#define	DE	020000
#define	AE	010000
#define	SE	04000
#define	NE	02000
#define	WE	01000
#define	S62	040
#define	IE	0400
#define	IM	020
#define	OL	04
#define	DMIDC0	0
#define	DMIDC1	01
#define	DMIDC2	02
#define	DMIDC3	03



struct {
	int sstat;
	int pstat;
	int badr;
	int wcr;
	};


struct devtab fdtab;


fdstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if (bp->b_blkno >= NFDBLK || bp->b_dev>=NFD || bp->b_dev<0) {
		bp->b_flags =| B_DONE | B_ERROR;
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
	register com;

	if((bp=fdtab.d_actf)==0)
		return;
	fdtab.d_active++;
	com=bp->b_dev<<11;
	while((FDADDR->pstat&DONE)==0);
	FDADDR->badr=bp->b_addr;
	FDADDR->wcr= -256;
	FDADDR->sstat=com|CMD1|(bp->b_blkno&017);
	com=| CMD0|((bp->b_blkno>>4)&0177);
	FDADDR->pstat=IENB;
	if(bp->b_flags&B_READ)
		FDADDR->sstat=RS|com;
	else
		FDADDR->sstat=WSC0|com;
	}



fdintr()
{
	register struct buf *bp;
	register int unit;

	if (fdtab.d_active == 0)
		return;
	bp = fdtab.d_actf;
	fdtab.d_active = 0;
	if(FDADDR->pstat&ERR){
		if (++fdtab.d_errcnt <= 10) {
			fdstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	fdtab.d_errcnt = 0;
	if((bp->b_wcount =+ 256) == 0) {
		fdtab.d_actf = bp->b_link;
		bp->b_flags =| B_DONE;
#ifdef BGOPTION
		wakeup(bp);
#endif
	} else {
		bp->b_addr =+ 512;
		bp->b_blkno++;
	}
	fdstart();
}

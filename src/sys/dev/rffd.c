/*
 * RF disk driver for LSX
 */
#include "param.h"
#include "buf.h"
#include "user.h"

#define RFADR  0177460
#define NFDBLK 1024

#define READY 0200
#define IENB 0100
#define RCOM 04
#define WCOM 02
#define GO 01

struct {
	int rfcs;
	int rfwc;
	int rfba;
	int rfda;
	int rfdae;
	};

struct devtab fdtab;
int fdstr, fdint;


fdstrategy( abp )
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
		fdtab.d_actl->b_link = bp;

	fdtab.d_actl = bp;
	if( fdtab.d_active == 0 )
		fdstart();
	spl0();
}

fdstart()
{

	register struct buf *bp;
	register int com;

	if( (bp = fdtab.d_actf) == 0)
		return;

	fdtab.d_active++ ;
	while( (RFADR->rfcs & READY) == 0);

	RFADR->rfba = bp->b_addr;
	RFADR->rfwc = bp->b_wcount;
	RFADR->rfdae = bp->b_blkno.hibyte;
	RFADR->rfda = bp->b_blkno << 8;

	if( bp->b_flags & B_READ )
		com = IENB | RCOM | GO;
	else
		com = IENB | WCOM | GO;

	RFADR->rfcs = com;
	fdstr++ ;
}

fdintr()
{

	register struct buf *bp;

	fdint++ ;
	if( fdtab.d_active == 0 )
		return;

	bp = fdtab.d_actf;
	fdtab.d_active = 0;

	if( RFADR->rfcs < 0 ) {
		if( ++fdtab.d_errcnt <= 10 ) {
			fdstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}

	fdtab.d_errcnt = 0;
	fdtab.d_actf = bp->b_link;
	bp->b_flags |= B_DONE;
#ifdef BGOPTION
	wakeup(bp);
#endif
	fdstart();
}

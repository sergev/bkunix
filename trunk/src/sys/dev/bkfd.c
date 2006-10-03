/*
 * FD/HDD firmware ROM interface for BK UNIX
 */
#include "param.h"
#include "buf.h"
#include "user.h"


struct devtab fdtab;
#define NFDBLK 1600

static char ioarea[64];

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

fdstart()
{

	register struct buf *bp;
	register char *r3, *r2;
	static struct buf * savbp;

	if( (bp = fdtab.d_actf) == 0)
		return;

	/* fdtab.d_active++ ; */
	fdtab.d_actf = (struct buf *) bp->b_link; /* unlink immediately */
	bp->b_flags |= B_DONE;
	savbp = bp;
	r3 = ioarea;
	r3[034] = bp->b_dev;

	do {
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
	} while (++fdtab.d_errcnt <= 10);

	fdtab.d_errcnt = 0;
	bp->b_flags != B_ERROR;
}

#if 0
fdintr()
{
	register struct buf *bp;

	fdint++ ;
	if( fdtab.d_active == 0 )
		return;

	bp = fdtab.d_actf;

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
#endif

void fdinit() {
	register char *r4, *r3;
	r3 = ioarea;
	((void(*)())0160010)();
}

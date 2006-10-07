/disk boot 

#include "param.h"

core = 12
rootdir = 1
rp = 0
rk = 0
rf = 0
rx = 1
hp = 0

.. = [core*2048]-512

reset = 5
large = 010000

start:
	mov	pc,r2
	tst	-(r2)
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	reset
1:
	mov	(r2)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)

	/output message to tty

2:
	.if	rx
		tst	r0		/ unit number is in r0 (0,1)
		jeq	0f
		bis	$020,readop	/ set instruct to read unit 1
0:
	.endif
	mov	$nm,r1
1:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putc
	br	1b

	/get command from tty

1:
	clr	r1
2:
	clr	r0
3:
	movb	r0,(r1)+
4:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'@
	beq	1b
	cmp	r0,$' 
	beq	2b
	cmp	r0,$'#
	bne	3b
	dec	r1
	bgt	4b
	br	1b

	/put command on stack in exec format

1:
	mov	sp,r3
	clrb	(r1)+
	clrb	(r1)+
	bic	$1,r1
	sub	r1,sp
	clr	r2
1:
	tst	r1
	beq	1f
	movb	-(r1),-(r3)
	beq	1b
2:
	mov	r3,r4
	movb	-(r1),-(r3)
	bne	2b
	mov	r4,-(sp)
	inc	r2
	br	1b
1:
	mov	r2,-(sp)

	/look up command path name

	.if	hp
		mov	$hpcs1,r0
		mov	$040,8(r0)	/drive clear
		mov	$021,(r0)	/preset
		mov	$010000,26(r0)	/fmt22
	.endif

	decb	-(r4)
	mov	$rootdir,in
1:
	jsr	pc,geti
	mov	r4,r3
	mov	$buf+512,r5
2:
	mov	r3,r4
	mov	r5,r0
	add	$16,r5
3:
	cmp	r0,$buf+512
	blo	4f
	jsr	pc,getblk
		br	start
	sub	$512,r5
4:
	cmp	r3,r4
	bne	5f
	mov	(r0)+,in
	beq	2b
5:
	tstb	(r4)+
	beq	1f
	cmpb	(r4),$'/
	beq	1b
	cmp	r0,r5
	bhis	5b
	cmpb	(r4),(r0)+
	beq	3b
	br	2b
1:
	jsr	pc,geti
/ was	clr	r3
	mov	$ BOTSYS, r3
1:
	jsr	pc,getblk
		br	start
	cmp	(r0),$0407
	bne	2f
	add	$020,r0
2:
	mov	(r0)+,(r3)+
	cmp	r3, $ TOPSYS
	beq	1f
	cmp	r0,$buf+512
	blo	2b
	jsr	pc,getblk
		br	1f
	br	2b
1:
	jsr	pc,*$ BOTSYS

geti:
	mov	in,r1
	add	$31,r1
	mov	r1,-(sp)
	asr	r1
	asr	r1
	asr	r1
	asr	r1
	jsr	pc,rblk
	mov	(sp)+,r1
	bic	$!017,r1
	asl	r1
	asl	r1
	asl	r1
	asl	r1
	asl	r1
	add	r0,r1
	mov	$inode,r0
1:
	mov	(r1)+,(r0)+
	cmp	r0,$addr+16
	blo	1b
	clr	r2
	rts	pc


getblk:
	add	$2,(sp)
	mov	r2,r0
	inc	r2
	bit	$large,mode
	bne	1f
	asl	r0
	mov	addr(r0),r1
	bne	rblk
2:
	sub	$2,(sp)
	clr	r0
	rts	pc
1:
	mov	r0,-(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r1
	beq	2b
	jsr	pc,rblk
	asl	(sp)
	bic	$!0776,(sp)
	add	(sp)+,r0
	mov	(r0),r1
	beq	2b

rpda = 0176724
rkda = 0177412
rfda = 0177466
rxda = 0177170
rblk:

	.if	rx
		mov	r5,-(sp)
		mov	r4,-(sp)
		mov	r3,-(sp)
		mov	r2,-(sp)
		mov	$rxda,r4
		mov	$rxda+2,r3
		mov	$buf,r2
		asl	r1
		asl	r1
		mov	r1,-(sp)	/ sectr = blockno * 4
		mov	r1,r5
		add	$4,r5		/ blkno*4 + 4
	L1:	bit	$040,(r4)
		beq	L1
		mov	(pc)+,(r4)	/ *ptcs = READ|GO|UNIT
	readop:	7
		mov	(sp),r1		/ now calculate sector,track
					/ sector = (sectr * 3) % 26 + 1
		asl	r1
		add	(sp),r1
		clr	r0
0:
		sub	$26,r1
		bmi	0f
		inc	r0
		br	0b
0:
		add	$27,r1
	L6:	tstb	(r4)		/while(ptcs->lobyte == 0);
		jeq	L6
		movb	r1,(r3)		/ ptdb->lobyte = sector
		mov	r0,r1
					/ track = sectr/26 + 1
		clr	r0
0:
		sub	$3,r1
		bmi	0f
		inc	r0
		br	0b
0:
		inc	r0
		cmp	$77, r0	/ if(sector == 77) sector = 0
		bne	L9
		clr	r0
	L9:	tstb	(r4)		/ while(ptcs->lobyte == 0);
		jeq	L9
		movb	r0,(r3)		/ ptcs->lobyte = track
	L11:	tstb	(r4)		/ while(ptcs->lobyte == 0);
		jeq	L11
		mov	$3,(r4)		/ *ptcs = EMPTY|GO
		jbr	L16		/ do {
	L2:	movb	(r3),(r2)+	/	while(ptcs->lobyte < 0)
	L16:	tstb	(r4)		/	    *ptbf++ = ptdb->lobyte
		jlt	L2		/ } while(ptcs->lobyte <= 0)
		tstb	(r4)
		jle	L16
		inc	(sp)		/ increment blkno and see if it is
		cmp	(sp),r5		/ less than blkno*4 + 4
		jlt	L1		/ yes--loop again.
		tst	(sp)+
		mov	(sp)+,r2
		mov	(sp)+,r3
		mov	(sp)+,r4
		mov	(sp)+,r5
		mov	$buf, r0
		rts	pc
	.endif

    .if    rx-1

	clr	r0

	.if	rp
		div	$10,r0
		mov	r1,-(sp)
		mov	r0,r1
		clr	r0
		div	$20,r0
		bisb	r1,1(sp)
		mov	$rpda,r1
		mov	(sp)+,(r1)
	.endif

	.if	rk
		div	$12,r0
		ash	$4,r0
		add	r1,r0
		mov	$rkda+2,r1
	.endif

	.if	rf
		ashc	$8,r0
		mov	r0,*$rfda+2
		ashc	$16,r0
		mov	$rfda+2,r1
	.endif

	hpcs1 = 0176700
hpda	= 0176706
hpdc	= 0176734
	.if	hp
		div	$22,r0
		mov	r1,-(sp)
		mov	r0,r1
		clr	r0
		div	$19,r0
		mov	r0,*$hpdc
		bisb	r1,1(sp)
		mov	(sp)+,r0
		mov	$hpda+2,r1
	.endif

	mov	r0,-(r1)
	mov	$buf,r0
	mov	r0,-(r1)
	mov	$-256,-(r1)

	.if	rf+rk+rp
		mov	$5,-(r1)
	.endif

	.if	hp
		mov	$071,-(r1)
	.endif

1:
	tstb	(r1)
	bge	1b
	rts	pc

    .endif

tks = 0177560
tkb = 0177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	tkb,r0
	bic	$!0177,r0
	cmp	r0,$0101
	blo	1f
	cmp	r0,$0132
	bhi	1f
	add	$040,r0
1:
	cmp	r0,$015
	bne	putc
	mov	$012,r0

tps = 0177564
tpb = 0177566
putc:
	tstb	tps
	bge	putc
	cmp	r0,$012
	bne	1f
	mov	$015,r0
	jsr	pc,putc
	mov	$0212,r0
	jsr	pc,putc
	clr	r0
	jsr	pc,putc
	mov	$012,r0
	rts	pc
1:
	mov	r0,tpb
	rts	pc

nm:
	.byte	012
	.if	rp
		<rp boot:\0>
	.endif

	.if	rk
		<rk boot:\0>
	.endif

	.if	rf
		<rf boot:\0>
	.endif

	.if  	hp
		<hp boot:\0>
	.endif

	.if	rx
		<rx boot:\0>
	.endif
	.even
in:	rootdir
end:

inode = ..-1024
mode = inode
addr = inode+8
buf = inode+32


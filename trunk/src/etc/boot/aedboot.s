/ floppy disk boot program to load and transfer
/ to lsx system.

/ entry is made by jsr pc,*$0
/ so return can be rts pc

/ interrupt vector for AED disk is filled with
/ address zero to take advantage of IPL mode

EIS = 0
AED = 1
DEC = 0
PER = 0
SYK = 0
inflg = 0
outflg = 1

core = 16.
.. = [core*2048.]-512.
start:
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	reset
	clr	r0
	br	0f
	0
	0
	0
	0
0:
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)

2:
	mov	$inod,r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
.if outflg
	mov	$'\n,r0
	jsr	pc,putc
.endif
	mov	$names,r1
	mov	$1,r0
1:
	clr	bno
	jsr	pc,iget
	tst	(r1)
	beq	1f
2:
	jsr	pc,rmblk
		br start
	mov	$buf,r2
3:
	mov	r1,r3
	mov	r2,r4
	add	$16.,r2
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r2
	blo	4b
	mov	-16.(r2),r0

.if	AED
	br	6f		/ make room for aed vector
. = 174^.
6:
.endif

	add	$14.,r1
	br	1b
5:
	cmp	r2,$buf+512.
	blo	3b
	br	2b
1:
	clr	r1
1:
	jsr	pc,rmblk
		br 1f
	mov	$buf,r2
2:
	mov	(r2)+,(r1)+
	cmp	r2,$buf+512.
	blo	2b
	br	1b
1:
	clr	r0
	cmp	(r0),$407
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	jsr	pc,*$0
	br	start

iget:
	add	$31.,r0
	mov	r0,r5
.if	EIS
	ash	$-4.,r0
.endif
.if	EIS-1
	asr	r0
	asr	r0
	asr	r0
	asr	r0
.endif
	jsr	pc,rblk
	bic	$!17,r5
.if	EIS
	ash	$5.,r5
.endif
.if	EIS-1
	asl	r5
	asl	r5
	asl	r5
	asl	r5
	asl	r5
.endif
	add	$buf,r5
	mov	$inod,r4
1:
	mov	(r5)+,(r4)+
	cmp	r4,$addr+16.
	blo	1b
	rts	pc

rmblk:
	add	$2,(sp)
	mov	bno,r0
	inc	bno
	bit	$CONT,mode
	beq	0f
	cmp	r0,addr+2
	bge	2f
	add	addr,r0
	br	rblk
0:
	bit	$LRG,mode
	bne	1f
	asl	r0
	mov	addr(r0),r0
	bne	rblk
2:
	sub	$2,(sp)
	rts	pc
1:
	clr	-(sp)
	movb	r0,(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r0
	beq	2b
	jsr	pc,rblk
	mov	(sp)+,r0
	asl	r0
	mov	buf(r0),r0
	beq	2b

rblk:

.if	PER
fdcont = 177600
fdstat = 177602
fdba = 177604

rtblk:
	tst	*$fdstat
	bpl	rtblk
	mov	$buf,*$fdba
	mov	r0,*$fdcont
0:
	tst	*$fdstat
	bpl	0b
	rts	pc
.endif

.if DEC
rxcs = 177170
rxdb = 177172

rtblk:
	mov	$buf,-(sp)	/ buffer address
	clr	-(sp)		/ sector count
	mov	$rxcs,r4
3:
	mov	$7,(r4)		/ read command
0:
	tstb	(r4)
	beq	0b
	mov	r0,r3		/ block number
	asl	r3
	asl	r3
	add	(sp),r3
	mov	r3,r2
	asl	r3
	add	r2,r3
	clr	r2
.if	EIS
	div	$26.,r2
.endif
.if	EIS-1
0:
	sub	$26.,r3
	bmi	0f
	inc	r2
	br	0b
0:
	add	$26.,r3
.endif
	inc	r3
	mov	r3,2(r4)	/ sector number
0:
	tstb	(r4)
	beq	0b
	mov	r2,r3
	clr	r2
.if	EIS
	div	$3.,r2
.endif
.if	EIS-1
0:
	sub	$3.,r3
	bmi	0f
	inc	r2
	br	0b
0:
.endif
	mov	r2,2(r4)	/ track number
0:
	bit	$40,(r4)	/ Done?
	beq	0b
	mov	2(sp),r2	/ buffer address
	mov	$3,(r4)		/ empty buffer command
0:
	bit	$40,(r4)	/ Done?
	bne	2f
	tstb	(r4)		/ Data ready?
	bpl	0b
	movb	2(r4),(r2)+
	br	0b
2:
	inc	(sp)		/ sector count
	add	$128.,2(sp)	/ buffer address
	cmp	(sp),$4
	blt	3b
	cmp	(sp)+,(sp)+
	rts	pc
.endif

.if	SYK
flfg = 176000
flcm = 176002
flst = 176004
flda = 176006

rtblk:
	mov	$buf,-(sp)	/ buffer address
	clr	-(sp)		/ sector count
	mov	$flcm,r4
3:
	mov	r0,r3		/ block number
	asl	r3
	asl	r3
	add	(sp),r3
	clr	r2
.if	EIS
	div	$26.,r2
.endif
.if	EIS-1
0:
	sub	$26.,r3
	bmi	0f
	inc	r2
	br	0b
0:
	add	$26.,r3
.endif
	inc	r3
	movb	r2,(r4)		/ track number
	movb	r3,(r4)		/ sector number
	mov	2(sp),r2	/ buffer address
	mov	$128.,r3	/ bytes per sector
0:
	tstb	*$flfg
	bpl	0b
	movb	*$flda,(r2)+
	dec	r3
	bpl	0b
	movb	$203,(r4)	/ Terminate command
0:
	bit	$4,*$flst	/ wait for busy to drop
	bne	0b
	inc	(sp)		/ sector count
	add	$128.,2(sp)	/ buffer address
	cmp	(sp),$4
	blt	3b
	cmp	(sp)+,(sp)+
	rts	pc
.endif

.if	AED
sstat = 164000
pstat = 164002
badr = 164004
wcr = 164006

rtblk:
	tstb	*$pstat
	bpl	rtblk
	mov	$buf,*$badr
	mov	$-256.,*$wcr
	mov r0,r3
	bic	$!17,r3
	bis	$1000,r3
	mov	r3,*$sstat
	mov	r0,r3
	ash	$-4.,r3
	bic	$!177,r3
	bis	$20000,r3
	mov	r3,*$sstat
0:
	tstb	*$pstat
	bpl	0b
	rts	pc
.endif

.if inflg
tks = 177560
tkb = 177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$40,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0
.endif

.if outflg
tps = 177564
tpb = 177566
putc:
	tstb	tps
	bge	putc
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,putc
	mov	$'\n+200,r0
	jsr	pc,putc
	clr	r0
	jsr	pc,putc
	mov	$'\n,r0
	rts	pc
1:
	mov	r0,tpb
	rts	pc
.endif

names:	<lsx\0>
	.=names+16.
end:
inod = ..-1024.
mode = inod
addr = inod+8.
buf = inod+32.
bno = buf+514.
rxblk = bno+2
LRG = 10000
CONT = 1000
reset = 5

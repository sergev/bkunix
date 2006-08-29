/
/	rxboot.p -- rx11 skeleton bootstrap program.
/	this program takes only one sector.
/	rxboot.p is stored in boot location
/		sector	1
/		track	1
/	pathname bootstrap is located in location
/		** see l_sectr table
/	this program assumes that the bootstrap rom
/	leaves the unit number (0,1) in register r0.
/	this program in turn leaves the unit number
/	(0,1) in register r0 when it jumps to the
/	pathname bootstrap program.
/
/	modified by ljh for new rx layout 2/10/77
rxcs	= 0177170
rxdb	= rxcs+2
go	= 1
empty	= 2
intlev	= 2
rdrx	= 6
unit_1	= 020
done	= 040
treq	= 0200
initrx	= 040000
halt	= 0
nop	= 0240
	.text
	.globl	rxboot,_rxboot
_rxboot:
rxboot:
	nop			/this is required by the rom
	tst	r0		/unit number is in r0 (0,1)
	beq	0f
	bis	$unit_1,readop	/set instruction to read unit 1
0:
	mov	$rxcs,r1	/control and status register
	mov	$rxdb,r2	/data and sector/track addr reg
	mov	$l_sctr, r4	/ set address of secter and track table
next:
	bit	$done,(r1)
	beq	next
	mov	(pc)+,(r1)	/read sector instruction
readop:
	.word	rdrx+go		/can be modified by unit number
1:
	tstb	(r1)		/transfer request flag
	beq	1b
	tstb	(r4)		/ last block ?
	beq	rxboot+0200	/ jump to program if so
	movb	(r4)+, (r2)	/ more to go. setup for next sector
2:
	tstb	(r1)		/transfer request flag
	beq	2b
	movb	(r4)+, (r2)	/ track address
3:
	tstb	(r1)		/read complete ?
	beq	3b
	bmi	erflag		/treq on -- error
	tst	(r1)		/error flag
	bmi	erflag
	mov	bufaddr,r0	/current buffer address
	mov	$empty+go,(r1)	/empty rx function
	br	1f
efloop:
	movb	(r2),(r0)+	/empty buffer
1:
	tstb	(r1)		/transfer request flag
	bmi	efloop		/br if ready
	beq	1b		/wait for flag
	tst	(r1)		/error flag
	bmi	erflag
	mov	r0,bufaddr	/next set of 'empty' locations
	clr	r0		/setup unit number in r0
	cmpb	$rdrx+go,readop	/is it zero
	adc	r0		/id unit number 0,1
	br	next		/read another sector
erflag:
	mov	$initrx,(r1)	/initialize heads -- read first sector
	halt			/error
	br	rxboot		/restart
l_sctr:
				/ table of block addresses for boot prog.
	.byte 4, 1		/ sector 4	track 1
	.byte 7, 1		/ sector 7	track 1
	.byte 10, 1		/ sector 10	track 1
	.byte 21, 0		/ sector 21	track 0
	.byte 24, 0		/ sector 24	track 0
	.word 0			/ end of table
bufaddr:
	.word 0200		/start of pathname bootstrap
	.even

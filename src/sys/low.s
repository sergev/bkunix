/ Copyright 1975 Bell Telephone Laboratories Inc
/ low core

br4 = 340
br5 = 340
br6 = 340
br7 = 340

. = 0^.
	br	1f
	4

/ trap vectors
	trap; br7+0.		/ bus error
.if	EIS
	trap; br7+1.		/ illegal instruction
.endif
.if	EIS-1
.globl	trapem
	trapem; br7+1.		/ emulation package
.endif
	trap; br7+2.		/ bpt-trace trap
	trap; br7+3.		/ iot trap
	trap; br7+4.		/ power fail
	trap; br7+5.		/ emulator trap
	trap; br7+6.		/ system entry

. = 40^.
.globl	start
1:	jmp	start
	jmp	dump
.if RXROM
	jmp	reboot
.endif

.if KL
. = 60^.
	klin; br4
	klou; br4
.endif

.if CLOCK
. = 100^.
	kwlp; br6
	kwlp; br6
.endif

.if PER
. = 124^.
	fdintr; br5
.endif

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call, trap

.if KL
.globl _klrint, _klxint
klin:	jsr	r0,call; _klrint
klou:	jsr	r0,call; _klxint
.endif

.if CLOCK
.globl _clock
kwlp:	jsr	r0,call; _clock
.endif

.globl	_fdintr
fdintr:	jsr	r0,call; _fdintr

.globl _panic
_panic:
dump:
	0

.if AED
. = 170^.
	fdintr; br5
.endif

.if PER
. = 174^.
	fdintr; br5
.endif

.if SYK
. = 174^.
	fdintr; br5
.endif

.if TVT
. = 200^.
	klin; br5
.endif

.if RF
. = 204^.
	fdintr; br5
.endif


.if DEC
.if EIS-1
.globl	_decmch

rxcs = 177170
rxdb = 177172

_decmch:
	mov	r2,-(sp)
	mov	$rxdb,r1
	mov	4(sp),r0
	asl	r0
	add	4(sp),r0
	clr	r2
0:
	sub	$26.,r0
	bmi	0f
	inc	r2
	br	0b
0:
	add	$26.,r0
	inc	r0
	mov	r0,(r1)	/ sector number
0:
	bit	$200,-2(r1)
	beq	0b
	mov	r2,r0
	clr	r2
0:
	sub	$3.,r0
	bmi	0f
	inc	r2
	br	0b
0:
.if IBMS
	inc	r2
	cmp	$77.,r2
	bne	0f
	clr	r2
0:
.endif
	mov	r2,(r1)	/ track number
	mov	(sp)+,r2
	rts	pc
.endif
. = 264^.
	fdintr; br5
.endif

.if FLTVECT
. = 300^.
	. = .+40
.endif

.if RXROM
reboot:
/
/	rxrom.p -- is similar to the DEC rom bootstrap
/	programs for the rx11 floppy disk units.
/
unit_0	=0
go	=1
empty	=2
rdrx	=6
unit_1	=20
done	=40
treq	=200
error	=100000
rxcs	=177170
/
	.globl	rxrom
/
rxrom:
		/can use unit_1 below to boot from unit 1
	mov	$error+treq+done+unit_0+rdrx+go,r2	/mask reg
x0:
	mov	$rxcs,r1	/pointer to status reg
x1:
	bitb	r2,(r1)		/test done bit
	jeq	x1
	movb	$7,r3		/3 bits for loop counter
	mov	r1,r0		/pointer - rxcs then rxdb
	mov	r2,(r0)+	/read sector command
	jbr	x3
x2:
	mov	$1,(r0)		/sector 1; track 1; nop
x3:
	asr	r3		/shift out bits for loop count
	jcs	x5		/jmp if r3 not zero
	movb	(pc)+,(r1)	/the following immediate operand
				/111023 looks like an empty function
				/023 == 3 == (empty + go)
x4:
	movb	(r0),(r3)+	/read a byte into core from zero
x5:
	bit	r2,(r1)		/error, "treq", done
	jeq	x5		/until ready
	jmi	x0		/error condition
	jcs	x2		/jmp if set by "x3:"
	tstb	(r1)		/is treq set
	jmi	x4		/jmp if treq high
	clr	r0		/pointer to location zero
	cmp	$240,(r0)	/nop instruction at location zero
	jne	x0		/retry if test fails
	cmpb	$treq+done+rdrx+go,r2	/clear carry if equal
	adc	r0		/id unit number 0,1 (see rxrom:)
	clr	pc		/execute the bootstrap
/
/	the following is the octal of the above program.
/	this program can be loaded anywhere after location 1000
/
/	012702
/	100247	/this can be 100267 for booting from unit 1
/	012701
/	177170
/	130211
/	001776
/	112703
/	000007
/	010100
/	010220
/	000402
/	012710
/	000001
/	006203
/	103402
/	112711
/	111023
/	030211
/	001776
/	100756
/	103766
/	105711
/	100771
/	005000
/	022710
/	000240
/	001347
/	122702
/	000247
/	005500
/	005007
/
/	end of hardware type rom program
/
/	.end rxrom.p
.endif

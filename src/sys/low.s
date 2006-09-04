/ Copyright 1975 Bell Telephone Laboratories Inc
/ low core

br4 = 0340
br5 = 0340
br6 = 0340
br7 = 0340

init:	br	1f
	.word	4

/ trap vectors
	.word	trap, br7+0	/ bus error
.if	EIS
	.word	trap, br7+1	/ illegal instruction
.else
.globl	trapem
	.word	trapem, br7+1	/ emulation package
.endif
	.word	trap, br7+2	/ bpt-trace trap
	.word	trap, br7+3	/ iot trap
	.word	trap, br7+4	/ power fail
	.word	trap, br7+5	/ emulator trap
	.word	trap, br7+6	/ system entry

. = init+040
.globl	start
1:	jmp	start
	jmp	dump
.if RXROM
	jmp	reboot
.endif

.if KL
. = init+060
	.word	klin, br4
	.word	klou, br4
.endif

.if CLOCK
. = init+0100
	.word	kwlp, br6
	.word	kwlp, br6
.endif

.if PER
. = init+0124
	.word	fdintr, br5
.endif

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call, trap

.if KL
.globl _klrint, _klxint
klin:	jsr	r0,call; .word _klrint
klou:	jsr	r0,call; .word _klxint
.endif

.if CLOCK
.globl _clock
kwlp:	jsr	r0,call; .word _clock
.endif

.globl	_fdintr
fdintr:	jsr	r0,call; .word _fdintr

.globl _panic
_panic:
dump:
	.word	0

.if AED
. = init+0170
	.word	fdintr, br5
.endif

.if PER
. = init+0174
	.word	fdintr, br5
.endif

.if SYK
. = init+0174
	.word	fdintr, br5
.endif

.if TVT
. = init+0200
	.word	klin, br5
.endif

.if RF
. = init+0204
	.word	fdintr, br5
.endif


.if DEC
.if EIS-1
.globl	_decmch

rxcs = 0177170
rxdb = 0177172

_decmch:
	mov	r2,-(sp)
	mov	$rxdb,r1
	mov	4(sp),r0
	asl	r0
	add	4(sp),r0
	clr	r2
0:
	sub	$26,r0
	bmi	0f
	inc	r2
	br	0b
0:
	add	$26,r0
	inc	r0
	mov	r0,(r1)	/ sector number
0:
	bit	$0200,-2(r1)
	beq	0b
	mov	r2,r0
	clr	r2
0:
	sub	$3,r0
	bmi	0f
	inc	r2
	br	0b
0:
.if IBMS
	inc	r2
	cmp	$77,r2
	bne	0f
	clr	r2
0:
.endif
	mov	r2,(r1)	/ track number
	mov	(sp)+,r2
	rts	pc
.endif
. = init+0264
	.word	fdintr, br5
.endif

.if FLTVECT
. = init+0300
	. = .+040
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
unit_1	=020
done	=040
treq	=0200
error	=0100000
rxcs	=0177170
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
	cmp	$0240,(r0)	/nop instruction at location zero
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

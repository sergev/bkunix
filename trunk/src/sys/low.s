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
. = init+0264
	.word	fdintr, br5
.endif

.if FLTVECT
. = init+0300
	. = .+040
.endif

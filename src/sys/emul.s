/mfps = 0106700^tst	/LSI instruction
/mtps = 0106400^tst

.if	EIS-1
.globl	nofault,emtrap,trapem

rti = 2
PS = 0177776

/
/
trapem:
.if LSI
	mfps	-4(sp)
.endif
.if LSI-1
	mov	*$PS,-4(sp)	/ trap type
.endif
	mov	r0,-(sp)
	tst	-(sp)
	mov	r1,-(sp)
	mov	$12,-(sp)
	add	sp,*sp
	mov	4(sp),-(sp)
	bic	$!17,*sp
	jsr	pc,trapems
	tst	(sp)+
	mov	(sp)+,r0	/ return stack pointer
	mov	(sp)+,r1
	tst	(sp)+
	add	$6,sp
	mov	-(sp),-(r0)
	mov	-(sp),-(r0)
	mov	-(sp),-(r0)
	mov	r0,sp
	mov	(sp)+,r0
	rti			/ return from emulation
/
/
/
/
/
/
/     EXTENDED INSTRUCTION SET EMULATOR FOR PDP 11/10
/
/
/   This program is designed to allow the use
/   of the PDP 11/40 instruction set on the PDP 11/10
/   in an environment which handles traps in the
/   same manner as the UNIX operating system.
/   This program returns the same results
/   and condition codes that the internal DEC
/   instruction does on the PDP 11/40.  This allows
/   programs which run under UNIX on an 11/40 to
/   be run on an 11/10 with a UNIX-like system.
/   The functions implimented are listed below:
/	multiply (MUL)
/	divide (DIV)
/	arithmetic shift (ASH)
/	arithmetic shift combined (ASHC)
/	exclusive OR (XOR)
/	subtract one and branch (SOB)
/
/
/
/
/   Upon entry to this program the stack is assumed
/   to look as follows:
/
/	< bottom portion of stack >
/	UPS  -- user's program status word at time of trap
/	UPC  -- user's program counter at time of trap
/	UR0  -- user's r0 at time of trap
/	---  -- not used by this program
/	UR1  -- user's r1
/	USP  -- user's stack pointer
/	DEV  -- trap type
/	RPC  -- return address to trap routine
/	< top of stack >
/
/
/
/
/   After it is determined that the trap is an illegal
/   instruction trap ( DEV = 1 ), a register save routine
/   is used which saves the other user registers and
/   allocates temporary space for variables.
/
/   This causes the stack to look as follows:
/
/	< bottom portion of stack >
/	UPS
/	UPC
/	UR0
/	---
/	UR1
/	USP
/	DEV
/	RPC
/	UR5 : user's r5    <---  register 5 points here
/	UR4 : user's r4
/	UR3 : user's r3
/	UR2 : user's r2
/	nfaddr : used to hold old nofault address
/	PCFLG :  used to hold value to increment UPC by
/	DSTN : used to hold address of destination field
/	MODMAP : used to hold interim condition codes
/	INST:	used to hold instruction being emulated <--- stack pointer
/	< top of stack >
/
/
/
/
/
UPC	=016
UPS	=020
MODMAP	=2
INST	=0
DSTN	=4
PCFLG	=6
nfaddr	=010
/
/
/
	.text
/
/
	.globl	csv, cret
/
/
/
emul2.o = .
/
trapems:
	jsr	r5,csv
	sub	$012,sp
/ set up to catch traps which occur during emululation
	mov	nofault,nfaddr(sp)
	mov	$errrtn,nofault
/ find instruction which caused trap
	mov	UPC(r5),r0
	mov	-(r0),(sp)
/ is it one to be emululated?
	cmp	(sp),$070000	/ 170000 for 40 testing
				/ 070000 for 10 use
	bgt	1f		/ bhis for 40 testing
				/ bgt for 10 use
	mov	(sp),r0
	bic	$077,r0
	cmp	r0,$06700
	beq	ssxt
	jmp	errrtn
ssxt:
	jsr	pc,getsrc
	bis	$4,(r4)	/set z-bit
	clr	*DSTN(sp)
	bit	$010,UPS(r5)	/n-bit on?
	beq	0f
	dec	*DSTN(sp)
	bic	$4,(r4)	/clear z-bit
0:
	mov	$4,r2
	jmp	setcc

1:
/
/
/
/
/ this routine gets the even-odd register pair
/   specified in the instruction and moves
/   them into r0 and r1
/
/ find register number = n
	mov	(sp),r4
	bic	$0177077,r4
	asl	r4
	asl	r4
	swab	r4
/ move register(n) into r0
	movb	regs(r4),r3
	add	r5,r3
	mov	(r3),r0
/ move register(n ORed with 1) into r1
	bis	$1,r4
	movb	regs(r4),r3
	add	r5,r3
	mov	(r3),r1
/
/
/ find out which function is to be performed
	mov	(sp),r4
	bic	$0170777,r4
	swab	r4
	jmp	*fcntbl(r4)
fcntbl:
	smul	/ multiply
	sdiv	/ divide
	sash	/ arithmetic shift
	sashc	/ arithmetic shift combined
	sxor	/ exclusive OR
	errrtn	/ unimplimented
	errrtn	/ unimplimented
	ssob	/ subtract one and branch
/
/
/
/ this routine performs the multiply function
/
smul:
	jsr	pc,getsrc
/ set up flags for proper sign management
	mov	r0,r1
	bge	1f
	neg	r1
	inc	r3
1:
	clr	r0
	tst	r2
	bge	1f
	neg	r2
	dec	r3
1:
/ set counter for 17 cycles
	mov	$021,-(sp)
2:
/ use shift and add algorithm
	clc
	ror	r0
	ror	r1
	bcc	1f
	add	r2,r0
1:
	dec	(sp)
	bne	2b
/ were signs the same?
	cmp	r3,(sp)+
	beq	1f
/ no, negate the product
	neg	r0
	neg	r1
	sbc	r0
1:
/ was product greater than 15 bits long?
	tst	r1
	bpl	1f
	cmp	$-1,r0
	bne	2f
	br	3f
1:
	tst	r0
	beq	3f
2:
/ if so, set the C bit
	bis	$1,*r4
3:
/ was the register odd?
	bit	$0100,(sp)
	beq	1f
/ if so, move product to r0
	mov	r1,r0
1:
/ set up flags and quit
	mov	$1,r3
	mov	$017,r2
	jmp	strreg
/
/
/
/
/ this routine performs the division function
/   the algorithm used is taken directly
/   from the DEC microcode flow in order
/   to make all condition codes and results
/   agree in all cases.
/
sdiv:
/ if an odd register instruction, error
	bit	(sp),$0100
	beq	1f
	jmp	errrtn
1:
	jsr	pc,getsrc
	mov	r5,-(sp)
	mov	r1,r5
/ is divisor 0?
	tst	r2
	bne	div5
/ if so, quit
	bis	$7,*r4
	br	div12
div5:
/ is dividend negative?
	tst	r0
	bge	div16
/ yes
	bis	$017,*r4
	neg	r0
	clr	r1
	sub	r5,r1
	sbc	r0
	bpl	div16
div12:
/ error exit
	bis	$2,*r4
	mov	(sp)+,r5
	mov	$017,r2
	jmp	setcc
div16:
/ first step of algorithm
	mov	r2,r3
	neg	r3
	clc
	rol	r1
	rol	r0
	tst	r2
	bmi	1f
	add	r3,r0
	br	2f
1:
	add	r2,r0
2:
	adc	r1
	bic	$1,*r4
/ test for overflow condition
	bit	$1,r1
	beq	1f
	tst	r0
	jne	div12
	clr	r5
	tst	r2
	bge	3f
	inc	r5
3:
	bit	$010,*r4
	beq	3f
	inc	r5
3:
	ror	r5
	jcc	div12
1:
/ if no overflow, interate algorithm 15 times
	mov	$017,-(sp)
div19:
	clc
	rol	r1
	rol	r0
	clr	r5
	tst	r2
	bpl	1f
	inc	r5
1:
	bit	$2,r1
	beq	1f
	inc	r5
1:
	ror	r5
	bcc	1f
	add	r3,r0
	br	2f
1:
	add	r2,r0
2:
	adc	r1
	dec	(sp)
	bne	div19
/ branch to appropriate cleanup and condition code routines
	cmp	r2,(sp)+
	bgt	1f
	bit	$1,r1
	beq	div31
	bne	div33
1:
	bit	$1,r1
	bne	div23
div27:
	add	r2,r0
div23:
	bit	$010,*r4
	bne	div29
div26:
	clr	*r4
	tst	r1
	beq	2f
	bpl	1f
	bis	$010,*r4
	br	1f
2:
	bis	$4,*r4
1:
	jmp	div42
div31:
	sub	r2,r0
div33:
	bit	$010,*r4
	beq	div37
	neg	r0
	jmp	div26
div29:
	neg	r0
div37:
	mov	r1,r3
	neg	r1
	com	r3
	jmi	div26
	clr	*r4
	tst	r1
	beq	2f
	bpl	1f
	bis	$010,*r4
	br	div42
2:
	bis	$4,*r4
1:
	bis	$2,*r4
div42:
/ exit procedure
	mov	(sp)+,r5
	mov	r0,r3
	mov	r1,r0
	mov	r3,r1
	mov	$1,r3
	mov	$-017,r2
	jmp	strreg
/
/
/
/
/ this routine performs the arithmetic shift function
/
sash:
	jsr	pc,getsrc
	br	1f
/
/
/
/
/ this routine performs the arithmetic shift combined function
/
sashc:
	jsr	pc,getsrc
	inc	r3
1:
/ get shift distance
	bic	$0177700,r2
	bit	$040,r2
	beq	1f
	bis	$0177700,r2
2:
/ right shift
	tst	r3
	beq	3f
/ for ashc
	asr	r0
	ror	r1
	br	4f
3:
/ for ash
	asr	r0
4:
	bic	$1,*r4
	bcc	5f
	bis	$1,*r4
5:
	inc	r2
	bne	2b
	br	9f
1:
/ left shift
	tst	r2
	beq	9f
1:
	tst	r3
	beq	3f
/ for ashc
	asl	r1
	rol	r0
	br	4f
3:
/ for ash
	asl	r0
4:
	bvc	2f
	bis	$2,*r4
2:
	bic	$1,*r4
	bcc	5f
	bis	$1,*r4
5:
	dec	r2
	bgt	1b
9:
	tst	r3
	bne	7f
/ for ash
	mov	r0,r1
	br	8f
7:
	bit	$0100,(sp)
	beq	8f
/ for ashc
	mov	r1,r0
8:
/ exit
	mov	$017,r2
	jmp	strreg
/
/
/
/
/ this routine performs the exclusive OR function
/   using A xor B = A and NOT B or B and NOT A
/
sxor:
	jsr	pc,getsrc
	mov	r0,r1
	bic	r2,r0	/ A AND NOT B
	bic	r1,r2	/ B AND NOT A
	bis	r0,r2	/ A XOR B
	mov	r2,*DSTN(sp)
	bvc	1f
	bis	$2,*r4
1:
	mov	r2,r0
	clr	r1
	mov	$016,r2
	jmp	setnz
/
/
/
/
/ this routine performs the subtract one and
/   branch function
/ ( untested )
/
ssob:
	mov	(sp),r2
	bic	$0177700,r2
/ subtract 1
	dec	r0
	beq	1f
/ modify PC
	asl	r2
	sub	r2,UPC(r5)
1:
/exit
	clr	r3
	clr	r2
	jmp	strreg
/
/
/
/
/ this routine performs the decoding and
/   fetching of the source operand and
/   places it in register 2
/
getsrc:
	mov	INST+2(sp),r4
	mov	r4,r3
	bic	$0177770,r4	/ register
	bic	$0177707,r3	/ mode
	asr	r3
	asr	r3
	clr	PCFLG+2(sp)
/ if PC addressing
	cmp	r4,$7
	bne	1f
	mov	$2,PCFLG+2(sp)
/ then modes 1,4 and 5 are errors
	cmp	r3,$2
	jeq	errrtn
	cmp	r3,$010
	jeq	errrtn
	cmp	r3,$012
	jeq	errrtn
1:
	asr	r3
	bic	$1,r3
	movb	regs(r4),r4
	add	r5,r4
	jmp	*mods(r3)
mods:
	0f	/ modes 0,1
	1f	/       2,3
	2f	/       4,5
	3f	/       6,7
0:
/ contents of register
	mov	r4,r2
	br	4f
1:
/ autoincrementing
	mov	(r4),r2
	add	$2,(r4)
	br	4f
2:
/ autodecrementing
	sub	$2,(r4)
	mov	(r4),r2
	br	4f
3:
/ indexed
	mov	(r4),r2
	mov	UPC(r5),r3
	add	$2,UPC(r5)
	add	PCFLG+2(sp),r2
	add	(r3),r2
4:
/ address of location in r2
/ is mode indirect?
	bit	$010,INST+2(sp)
	beq	5f
/ if so, new address to r2
	mov	(r2),r2
5:
/ effective address to DSTN
	mov	r2,DSTN+2(sp)
/ contents to r2
	mov	(r2),r2
	clr	r3
	mov	$MODMAP+2,r4
	add	sp,r4
	clr	*r4
	rts	pc

/
/
/
/
/ this table contains the offset in bytes of the
/   user's registers in the stack based at r5
/
	.data
regs:
	.byte	14	/ r0
	.byte	10	/ r1
	.byte	-6	/ r2
	.byte	-4	/ r3
	.byte	-2	/ r4
	.byte	 0	/ r5
	.byte	 6	/ r6
	.byte	16	/ r7
	.text
/
/
/
/
/ this routine stores the results of the operation
/   into the proper registers
/
strreg:
/ find register number = n
	mov	(sp),r4
	bic	$0177077,r4
	asl	r4
	asl	r4
	swab	r4
/ single (r0) or double (r0-r1) register store
	tst	r3
	beq	1f
/store r1 in r(n ORed with 1)
	mov	r4,r3
	bis	$1,r3
	movb	regs(r3),r3
	add	r5,r3
	mov	r1,(r3)
1:
/ store r0 in r(n)
	movb	regs(r4),r3
	add	r5,r3
	mov	r0,(r3)
/ branch to next routine by flag
	tst	r2
	bge	setnz
	neg	r2
	br	setcc
/
/
/
/
/ this routine cause a return to the regular
/   trap handler if an error is found so a normal
/   illegal instruction trap can be signalled
/
errrtn:
	mov	nfaddr(sp),nofault
	sub	$6,r5
	mov	r5,sp
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	add	$6,sp
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	jmp	emtrap
/
/
/
/
/ this routine sets the condition codes for
/   the n and z bits
/
setnz:
	tst	r0
	bgt	setcc
	blt	1f
	tst	r1
	bne	setcc
	bis	$4,MODMAP(sp)
	br	setcc
1:
	bis	$010,MODMAP(sp)
/
/
/
/
/ this routine sets the proper condition codes
/   in the user's PS and jumps to cret which
/   cleans up the stack and causes the
/   return to the user
/
setcc:
	bic	r2,UPS(r5)
	com	r2
	bic	r2,MODMAP(sp)
	bis	MODMAP(sp),UPS(r5)
	mov	nfaddr(sp),nofault
	jmp	cret
/
/
/
/
.endif

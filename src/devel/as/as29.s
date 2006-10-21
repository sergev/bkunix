/
/ copyright 1972 bell telephone laboratories inc.
/

/ as9 -- PDP-11 assembler pass 2

eae = 0

	.data
symtab:

/ special variables

dotrel: 02; dot:000000 /.
 01; dotdot:000000 /..

/ register

024;000000 /r0
024;000001 /r1
024;000002 /r2
024;000003 /r3
024;000004 /r4
024;000005 /r5
024;000006 /sp
024;000007 /pc


.if eae
/eae & switches

01;0177570 /csw
01;0177300 /div
01;0177302 /ac
01;0177304 /mq
01;0177306 /mul
01;0177310 /sc
01;0177311 /sr
01;0177312 /nor
01;0177314 /lsh
01;0177316 /ash

.endif

/ system calls

01;0000001 /exit
01;0000002 /fork
01;0000003 /read
01;0000004 /write
01;0000005 /open
01;0000006 /close
01;0000007 /wait
01;0000010 /creat
01;0000011 /link
01;0000012 /unlink
01;0000013 /exec
01;0000014 /chdir
01;0000015 /time
01;0000016 /makdir
01;0000017 /chmod
01;0000020 /chown
01;0000021 /break
01;0000022 /stat
01;0000023 /seek
01;0000024 /tell
01;0000025 /mount
01;0000026 /umount
01;0000027 /setuid
01;0000030 /getuid
01;0000031 /stime
01;0000034 /fstat
01;0000036 /mdate
01;0000037 /stty
01;0000040 /gtty
01;0000042 /nice
01;0000060 /signal

/ double operand

013;0010000 /mov
013;0110000 /movb
013;0020000 /cmp
013;0120000 /cmpb
013;0030000 /bit
013;0130000 /bitb
013;0040000 /bic
013;0140000 /bicb
013;0050000 /bis
013;0150000 /bisb
013;0060000 /add
013;0160000 /sub

/ branch

06;0000400 /br
06;0001000 /bne
06;0001400 /beq
06;0002000 /bge
06;0002400 /blt
06;0003000 /bgt
06;0003400 /ble
06;0100000 /bpl
06;0100400 /bmi
06;0101000 /bhi
06;0101400 /blos
06;0102000 /bvc
06;0102400 /bvs
06;0103000 /bhis
06;0103000 /bec
06;0103000 /bcc
06;0103400 /blo
06;0103400 /bcs
06;0103400 /bes

/ jump/ branch type

035;0000400 /jbr
036;0001000 /jne
036;0001400 /jeq
036;0002000 /jge
036;0002400 /jlt
036;0003000 /jgt
036;0003400 /jle
036;0100000 /jpl
036;0100400 /jmi
036;0101000 /jhi
036;0101400 /jlos
036;0102000 /jvc
036;0102400 /jvs
036;0103000 /jhis
036;0103000 /jec
036;0103000 /jcc
036;0103400 /jlo
036;0103400 /jcs
036;0103400 /jes

/ single operand

015;0005000 /clr
015;0105000 /clrb
015;0005100 /com
015;0105100 /comb
015;0005200 /inc
015;0105200 /incb
015;0005300 /dec
015;0105300 /decb
015;0005400 /neg
015;0105400 /negb
015;0005500 /adc
015;0105500 /adcb
015;0005600 /sbc
015;0105600 /sbcb
015;0005700 /tst
015;0105700 /tstb
015;0006000 /ror
015;0106000 /rorb
015;0006100 /rol
015;0106100 /rolb
015;0006200 /asr
015;0106200 /asrb
015;0006300 /asl
015;0106300 /aslb
015;0000100 /jmp
015;0000300 /swab

/ jsr

07;0004000 /jsr

/ rts

010;000200 /rts

/ simple operand

011;0104400 /sys

/ flag-setting

01;0000241 /clc
01;0000242 /clv
01;0000244 /clz
01;0000250 /cln
01;0000261 /sec
01;0000262 /sev
01;0000264 /sez
01;0000270 /sen

/ floating point ops

01;0170000 / cfcc
01;0170001 / setf
01;0170011 / setd
01;0170002 / seti
01;0170012 / setl
015;0170400 / clrf
015;0170700 / negf
015;0170600 / absf
015;0170500 / tstf
012;0172400 / movf
014;0177000 / movif
05;0175400 / movfi
014;0177400 / movof
05;0176000 / movfo
014;0172000 / addf
014;0173000 / subf
014;0171000 / mulf
014;0174400 / divf
014;0173400 / cmpf
014;0171400 / modf
014;0176400 / movie
05;0175000 / movei
015;0170100 / ldfps
015;0170200 / stfps
024;000000 / fr0
024;000001 / fr1
024;000002 / fr2
024;000003 / fr3
024;000004 / fr4
024;000005 / fr5

/ 11/45 operations

030;072000 /als (ash)
030;073000 /alsc (ashc)
030;070000 /mpy
.if eae-1
030;070000/ mul
030;071000 / div
030;072000 / ash
030;073000 /ashc
.endif
030;071000 /dvd
07;074000 /xor
015;006700 /sxt
011;006400 /mark
031;077000 /sob

/ specials

016;000000 /.byte
020;000000 /.even
021;000000 /.if
022;000000 /.endif
023;000000 /.globl
025;000000 /.text
026;000000 /.data
027;000000 /.bss
032;000000 /.comm

start:
	cmp	(sp),$4
	bge	1f
	jmp	aexit
1:
	cmp	(sp)+,$5
	blt	1f
	mov	$040,defund		/ globalize all undefineds
1:
	tst	(sp)+
	mov	(sp)+,a.tmp1
	mov	(sp)+,a.tmp2
	mov	(sp)+,a.tmp3
	jsr	r5,ofile; a.tmp1
	movb	r0,txtfil
	jsr	r5,ofile; a.tmp2
	movb	r0,fbfil
	jsr	r5,ofile; a.tmp3
	movb	r0,symf
	movb	r0,fin
	sys	creat; a.out; 0
	bec	1f
	jsr	r5,filerr; a.outp
1:
	movb	r0,fout
	jmp	go

/ overlaid buffer
inbuf	= start
.	= inbuf+512

/*
 * Disk boot.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"

#ifdef BK0011
#define PUTC	*0140156
#define GETC	*0140076
#else
#define PUTC	*$0102234
#define GETC	*$0101010
#endif
#define DISKIO	*$0160004

rootdir	=	1
inode	=	010000
large	=	010000
mode	=	inode
addr	=	inode+8
buf	=	inode+32

..	=	01000

inum:				/ use this word later as inode number
	.word	0240		/ nop is the "bootable" mark
start:
	clr	*$0177130	/ stop floppy motor
#ifdef BK0011
	mov	$0x0104,r0
	jsr	pc,*0140034	/ map page 4 at 0100000
#endif

	/output message to tty (using the monitor)
	mov	$nm,r1
1:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,PUTC
	br	1b

	/get command from tty (using the monitor)
1:
	mov	$1,r1
2:
	dec	r1
	beq	3f
	mov	$030,r0
0:	jsr	pc,PUTC
	sob	r1,0b		/ erase all input
3:	clr	r0
3:
	movb	r0,(r1)+
	cmp	$040, r1
	beq	start		/ string too long
4:
	jsr	pc,GETC
	cmp	r0,$025		/ ^U = kill
	beq	2b
	jsr	pc,PUTC
	cmp	r0,$'\n
	beq	go
	cmp	r0,$030		/ ^X = backspace
	bne	3b
	dec	r1
	bgt	4b
	br	1b

	/put command on stack in exec format
go:
	mov	$'\r,r0
	jsr	pc,PUTC

	cmp	$1,r1
	bne	2f
	clr	r1		/ empty string - boot bkunix
	mov	$bkunix,r0
	mov	$4,r2
1:	mov	(r0)+,(r1)+
	sob	r2,1b
2:
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

	decb	-(r4)
	mov	$rootdir,inum
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
	mov	(r0)+,inum
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
	mov	$ BOTSYS, r3
1:
	jsr	pc,getblk
		br	start
	cmp	(r0),$0407
	bne	2f
	add	$020,r0
2:
	mov	(r0)+,(r3)+
	cmp	r3, $ TOPSYS	/ do not overfill
	beq	1f
	cmp	r0,$buf+512
	blo	2b
	jsr	pc,getblk
		br	1f
	br	2b
1:
	mov	$02000, r3	/ use floppy i/o area at 02000
	movb	034(r3), r0	/ boot dev num
	jmp	*$ BOTSYS

/-----------------------
geti:
	mov	inum,r1
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

/-----------------------
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
rblk:
	mov	r5,-(sp)
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	mov	r1, r0		/ block num
	mov	$256, r1	/ word count, >0 - read
	mov	$buf, r2	/ read addr
	mov	$02000, r3	/ use floppy i/o area at 02000
	jsr	pc,DISKIO
	bcc	2f

	mov	$'!, r0		/ print error message
	jsr	pc,PUTC
	movb	*$052, r0	/ err code
	mov	r0, r1
	asr	r0
	asr 	r0
	asr	r0
	bic	$!1, r0
	bis	$060, r0
	jsr     pc,PUTC
	mov	r1, r0
	bic	$!7, r0
	bis	$060, r0
	jsr	pc,PUTC
	jbr     start
2:
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	mov	$buf, r0
	rts	pc

bkunix: <\0bkunix\0>

nm:	<\r\n>
#ifdef BK0011
	.byte	033, '1
#endif
	<boot: \0>

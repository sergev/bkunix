/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * C register save and restore routines.  When a C routine is called its stack
 * frame (after csv or ovhandlr have set things up) looks like this:
 *
 *	_________________________________
 *	| return address to caller 	|
 *	|-------------------------------|
 * r5->	| old r5 ("frame pointer")	|
 *	|-------------------------------|
 *	| old r4			|
 *	|-------------------------------|
 *	| old r3			|
 *	|-------------------------------|
 *	| old r2			|
 *	|-------------------------------|
 * sp->	| empty parameter slot		|
 *	|_______________________________|
 *
 * The "empty parameter slot" is a simple optimization the compiler uses to
 * avoid overhead in its generated parameter cleanup code after a function
 * has been called.  Rather than (for example) generating:
 *
 *	mov	foo,-(sp)	/ push parameter
 *	jsr	pc,bar		/ call function
 *	tst	(sp)+		/ throw away parameter
 *
 * The compiler always keeps one empty slot on the stack and generates:
 *
 *	mov	foo,(sp)	/ pass parameter
 *	jsr	pc,bar		/ call function
 *
 * The savings are not quite so dramatic when more than one parameter is
 * passed, but still worth the effort.  If the function has any local stack
 * variables, space for them will have to be allocated by the function thereby
 * "moving" the empty parameter slot down.
 */
#include "DEFS.h"

.globl	csv
csv:
	mov	r5,r0		/ save transfer address
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl	cret
cret:
	mov	r5,r1
	mov	-(r1),r4	/ restore registers, reset stack,
	mov	-(r1),r3	/ pop frame pointer and return
	mov	-(r1),r2
	mov	r5,sp		/ (more interrupt problems here *sigh* ...)
	mov	(sp)+,r5
	rts	pc

/*
 * BK-0010 console driver
 *
 * Copyright 2006 Leonid Broukhis
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "user.h"
#include "systm.h"
#include "inode.h"
#include "file.h"
#include "reg.h"
#include "tty.h"
#include "proc.h"

/* console base address */
#define	KLADDR	((struct klregs*) 0177660)

/* BIOS variables */
#ifndef BK0011
#define	LIMIT	(*(int*)0164)
#define	BASE	(*(int*)0202)
#define	OFFSET	(*(int*)0204)
#define	VSIZE	(*(int*)0206)
#define	WRKSIZE	(*(int*)0210)
#define	EXTRAM	(*(char*)042)
#endif

struct	tty tty;
static char cursoff;

void ttstart();

struct klregs {
	int klrcsr;
	int klrbuf;
	int scroll;
};

static char outbuf[64];
static char nch;

static void
bksend()
{
	asm("mov r5, -(sp)");
#ifdef BK0011
	outbuf[nch] = 0;
	asm("mov $_outbuf, r0");
	asm("jsr pc, *0140162");
#else
	asm("mov $_outbuf, r1");
	asm("movb _nch, r2");
	asm("jsr pc, *$0107050");
#endif
	asm("mov (sp)+, r5");
}

/*
 * When called with c == 0, performs a flush,
 * also flushes an \n.
 */
static void
putbuf(c)
	register int c;
{
	register int sps;

	sps = spl7();
	if (! c) {
		if (! nch)
			return;
		goto doit;
	}
	outbuf[nch++] = c;
	if (nch == sizeof(outbuf) - 1)
		goto many;

	if (c == '\n') {
doit:		if (nch == 1)
			ttputc(outbuf[0]);
		else {
many:			bksend();
		}
		nch = 0;
	}
	splx(sps);
}

/*
 * flush all TTY queues
 */
void
flushtty()
{
	register int sps;

	while (getc(&tty.t_canq) >= 0);
	nch = 0; /* kill output queue */
	wakeup(&tty.t_rawq);
	sps = spl7();
	while (getc(&tty.t_rawq) >= 0);
	tty.t_delct = 0;
	splx(sps);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
void
wflushtty()
{
	spl7();
	putbuf(0); /* flush output queue to screen */
	flushtty();
	spl0();
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
void
ttyoutput(c)
	register int c;
{
	register char *colp;

	if (!c) return;
	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t') {
		do {
			ttyoutput(' ');
		} while (tty.t_col&07);
		return;
	}

	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (tty.t_flags & CRMOD)
		if (c=='\n')
			ttyoutput('\r');
	putbuf(c);
	colp = &tty.t_col;
	switch (c) {

	/* ordinary */
	default:
		(*colp)++;
		break;

	/* backspace */
	case CERASE:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case '\n':
		*colp = 0;
		break;

	/* carriage return */
	case '\r':
		*colp = 0;
	}
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
void
ttyinput()
{
	register int c;
	register int flags;
	static int last;

	c = KLADDR->klrbuf;
	if (c != last) {
		keypress = 0;
		last = c;
	}
	flags = tty.t_flags;
	c &= 0177;
	if ((flags & CRMOD) && c == '\r')
		c = '\n';
	if (c==CQUIT) {
		signal(SIGQIT);
		goto flush;
	}
 	if (c==CINTR) {
		signal(SIGINT);
		goto flush;
	}
	if (tty.t_rawq.c_cc>=TTYHOG) {
	flush:
		flushtty();
		return;
	}
	putc(c, &tty.t_rawq);
	if (c=='\n' || c==004) {
		wakeup(&tty.t_rawq);
		if (putc(0377, &tty.t_rawq)==0)
			tty.t_delct++;
	}
	if (flags & ECHO) {
		ttyoutput(c == CKILL ? '\n' : c);
		putbuf(0); /* flush */
	}
#ifdef CLOCKOPT
	/* Set up time for autorepeat */
	c = *(int*)0177710;
	if (keypress) /* continuing repeat */
		c -= 18; /* about 1/20 sec */
	else	/* initial repeat */
		c -= 180; /* about 1/2 sec */
	if (!c) c--; /* avoid zero */
	keypress = c;
#endif
}

void
klopen()
{
	if ((tty.t_modes & TOPEN) == 0) {
		tty.t_modes |= TOPEN;
		tty.t_flags = ECHO|CRMOD;
	}
	KLADDR->klrcsr = 0;	/* enabling interrupts is inverted in BK */
}

void
klclose()
{
	wflushtty();
}

void
fullscr()
{
#ifndef BK0011
	LIMIT = 03000;
	BASE = 040000;
	VSIZE = 040000;
	WRKSIZE	= 036000;
	OFFSET = 032000;
	EXTRAM = 0;
	memzero(040000, 030000);
	memzero(077000, 01000);	/* erase the horizontal bar */
	KLADDR->scroll = 01230;
#endif
}

void
klsgtty(f)
	int f;
{
	register int *a;

	a = (int*) u.u_arg[0];
	a += 2;
	wflushtty();
	if (bad_user_address (a))
		return;
	if (f)
		tty.t_flags = *a;
	else
		*a = tty.t_flags;
}

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
int
canon()
{
	register char *bp;
	char *bp1;
	register int c;

	spl7();
	while (tty.t_delct==0) {
		sleep(&tty.t_rawq, TTIPRI);
	}
	spl0();
loop:
	bp = &canonb[2];
	while ((c=getc(&tty.t_rawq)) >= 0) {
		if (c==0377) {
			tty.t_delct--;
			break;
		}
		if (bp[-1]!='\\') {
			if (c == CERASE) {
				if (bp > &canonb[2])
					bp--;
				continue;
			}
			if (c == CKILL)
				goto loop;
			if (c==CEOT)
				continue;
		}
		*bp++ = c;
		if (bp>=canonb+CANBSIZ)
			break;
	}
	bp1 = bp;
	bp = &canonb[2];
	while (bp<bp1)
		putc(*bp++, &tty.t_canq);
	return(1);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 * The pc is backed up for the duration of this call.
 * In case of a caught interrupt, an RTI will re-execute.
 */
void
ttread()
{
	register char * base;
	register int n;

	if (cursoff) {
#ifdef BK0011
		putbuf(033);
		putbuf(070);
#else
		putbuf(0232);
#endif
		cursoff = 0;
	}
	putbuf(0);
	if (tty.t_canq.c_cc || canon()) {
		n = u.u_count;
		base = u.u_base;
		while (n && tty.t_canq.c_cc) {
			*base++ = getc(&tty.t_canq);
			--n;
		}
		u.u_base = base;
		u.u_offset += u.u_count - n;
		u.u_count = n;
	}
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
void
ttwrite()
{
	register unsigned char *base;
	register int n;

	if (! cursoff) {
#ifdef BK0011
		putbuf(033);
		putbuf(071);
#else
		putbuf(0232);
#endif
		cursoff++;
	}
	base = (unsigned char*) u.u_base;
	n = u.u_count;
	u.u_offset += n;
	while (n--) {
		ttyoutput(*base++);
	}
	u.u_count = 0;
	u.u_base = (char*) base;
}

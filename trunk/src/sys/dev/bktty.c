/*
 * BK-0010 console driver
 *
 * Copyright 2006 Leonid Broukhis
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
#define	NKL11	1
#define DSRDY	02
#define	RDRENB	01

struct	tty kl11[NKL11];
static char cursoff;

void ttstart();

struct klregs {
	int klrcsr;
	int klrbuf;
};

static char outbuf[64];
static char nch;

static void bksend() {
	asm("mov $_outbuf, r1");
	asm("movb _nch, r2");
	asm("mov r5, -(sp)");
	asm("jsr pc, *$0107050");
	asm("mov (sp)+, r5");
}

/*
 * When called with c == 0, performs a flush,
 * also flushes an \n.
 */
static void putbuf(c) 
int c;
{
	register int sps;
	sps = spl7();
	if (c)
		outbuf[nch++] = c;
	else if (nch)
		goto doit;
	else
		return;
	if (nch == 64)
		goto many;
	if (c == '\n') {
	doit:
		if (nch == 1)
			ttputc(outbuf[0]);
		else {
		many:
			bksend();
		}
		nch = 0;
	}
	rstps(sps);
}

/*
 * flush all TTY queues
 */
void
flushtty()
{
	register struct tty *tp;
	register int sps;

	tp = kl11;
	while (getc(&tp->t_canq) >= 0);
	nch = 0; /* kill output queue */
	wakeup(&tp->t_rawq);
	sps = spl7();
	while (getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	rstps(sps);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
void
wflushtty()
{
	register struct tty *tp;

	tp = kl11;
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
ttyoutput(ac)
	int ac;
{
	register int c;
	register struct tty *rtp;
	register char *colp;

	rtp = kl11;
	c = ac&0177;
	if (!c) return;
	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t') {
		do
			ttyoutput(' ');
		while (rtp->t_col&07);
		return;
	}

	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if(rtp->t_flags & CRMOD)
		if (c=='\n')
			ttyoutput('\r');
	putbuf(c);
	colp = &rtp->t_col;
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
ttyinput(ac)
	int ac;
{
	register int c;
	register struct tty *tp;
	register int flags;

	tp = kl11;
	c = ac;
	flags = tp->t_flags;
	c &= 0177;
	if(flags & CRMOD) if(c == '\r')
		c = '\n';
	if (c==CQUIT || c==CINTR) {
		signal(c==CINTR? SIGINT:SIGQIT);
		flushtty();
		return;
	}
	if (tp->t_rawq.c_cc>=TTYHOG) {
		flushtty();
		return;
	}
	putc(c, &tp->t_rawq);
	if (c=='\n' || c==004) {
		wakeup(&tp->t_rawq);
		if (putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
	}
	if(flags & ECHO) {
		ttyoutput(c == CKILL ? '\n' : c);
		putbuf(0); /* flush */
	}
}

void
klopen()
{
	register struct tty *tp;

	tp = &kl11[0];
	if((tp->t_modes & TOPEN) == 0) {
		tp->t_modes |= TOPEN;
		tp->t_flags = ECHO|CRMOD;
	}
	KLADDR->klrcsr = 0;	/* enabling interrupts is inverted in BK */
}

void
klclose()
{
	wflushtty();
}

void
klrint()
{
	register int c;

	c = KLADDR->klrbuf;
	ttyinput(c);
}

void
klsgtty(f)
	int f;
{
	register struct tty *tp;
	register int *a;

	tp = &kl11[0];
	a = (int*) u.u_arg[0];
	a += 2;
	wflushtty(tp);
	if (bad_user_address (a))
		return;
	if(f)
		tp->t_flags = *a;
	else
		*a = tp->t_flags;
}

/*
 * general TTY subroutines
 */

/*
 * The actual structure of a clist block manipulated by
 * getc and putc (mch.s)
 */
struct cblock {
	struct cblock *c_next;
	char info[6];
};

/*
 *  The character lists-- space for 6*NCLIST characters if lucky,
 * or 6 less otherwise: getc/putc needs the structs to be 8-aligned;
 * cinit() takes care of that.
 */
struct cblock cfree[NCLIST];

/* List head for unused character blocks. */
struct cblock *cfreelist;

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
void
cinit()
{
	register struct cblock *cp;
	cp = cfree;
	cp = (struct cblock*) (((int)cp+7)&~7);
	for (; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
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
	register struct tty *tp;
	register int c;

	tp = kl11;
	spl7();
	while (tp->t_delct==0) {
		sleep(&tp->t_rawq, TTIPRI);
	}
	spl0();
loop:
	bp = &canonb[2];
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if (c==0377) {
			tp->t_delct--;
			break;
		}
		if (bp[-1]!='\\') {
			if(c == CERASE) {
				if (bp > &canonb[2])
					bp--;
				continue;
			}
			if(c == CKILL)
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
		putc(*bp++, &tp->t_canq);
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
	register struct tty *tp;
	register char * base;
	register int n;

	if (cursoff) {
		putbuf(0232);
		cursoff=0;
	}
	putbuf(0);
	tp = kl11;
	if (tp->t_canq.c_cc || canon(tp)) {
		n = u.u_count;
		base = u.u_base;
		while (n && tp->t_canq.c_cc) {
			*base++ = getc(&tp->t_canq);
			--n;
		}
		u.u_base = base;
		dpadd(u.u_offset, u.u_count-n);
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
	register char *base;
	register int n;
	if (!cursoff) {
		putbuf(0232);
		cursoff++;
	}
	base = u.u_base;
	n = u.u_count;
	dpadd(u.u_offset, n);
	while (n--) {
		ttyoutput(*base++);
	}
	u.u_count = 0;
	u.u_base = base;
}

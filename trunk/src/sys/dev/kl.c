/*
 * KL/DL-11 driver
 *
 * Copyright 1975 Bell Telephone Laboratories Inc
 */
#include "param.h"
#include "user.h"
#include "systm.h"
#include "inode.h"
#include "file.h"
#include "reg.h"
#include "tty.h"
#include "proc.h"

/* base address */
#define	KLADDR	((struct klregs*) 0177560)	/* console */
#define	NKL11	1
#define DSRDY	02
#define	RDRENB	01

struct	tty kl11[NKL11];

struct klregs {
	int klrcsr;
	int klrbuf;
	int kltcsr;
	int kltbuf;
};

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
	while (getc(&tp->t_outq) >= 0);
	wakeup(&tp->t_rawq);
	wakeup(&tp->t_outq);
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
	while (tp->t_outq.c_cc) {
		sleep(&tp->t_outq, TTOPRI);
	}
	flushtty();
	spl0();
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 * If the SSTART bit is off for the tty the work is done here,
 * using the protocol of the single-line interfaces (KL, DL, DC);
 * otherwise the address word of the tty structure is
 * taken to be the name of the device-dependent startup routine.
 */
void
ttstart()
{
	register int c;
	register struct tty *tp;

	tp = kl11;
	if ((KLADDR->kltcsr&DONE)==0)
		return;
	if ((c=getc(&tp->t_outq)) >= 0) {
		KLADDR->kltbuf = c;
	}
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
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	colp = "({)}!|^~'`";
	if(rtp->t_flags & LCASE) {
		while(*colp++)
			if(c == *colp++) {
				ttyoutput('\\');
				c = colp[-2];
				break;
			}
		if ('a'<=c && c<='z')
			c += 'A' - 'a';
	}

	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if(rtp->t_flags & CRMOD)
		if (c=='\n')
			ttyoutput('\r');
	if (putc(c, &rtp->t_outq))
		return;

	colp = &rtp->t_col;
	switch (c) {

	/* ordinary */
	default:
		(*colp)++;
		break;

	/* backspace */
	case 010:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case '\n':
		*colp = 0;
		break;


	/* carriage return */
	case '\r':
		ttyoutput(0177);
		ttyoutput(0177);
		ttyoutput(0177);
		ttyoutput(0177);
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
	if(flags & LCASE)
		if(c>='A' && c<='Z')
			c += 'a'-'A';
	putc(c, &tp->t_rawq);
	if (c=='\n' || c==004) {
		wakeup(&tp->t_rawq);
		if (putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
	}
	if(flags & ECHO) {
		ttyoutput(c);
		ttstart();
	}
}

void
klopen()
{
	register struct tty *tp;

	tp = &kl11[0];
	if((tp->t_modes & TOPEN) == 0) {
		tp->t_modes |= TOPEN;
		tp->t_flags = ECHO|CRMOD|LCASE;
	}
	KLADDR->klrcsr |= IENABLE|DSRDY|RDRENB;
	KLADDR->kltcsr |= IENABLE;
}

void
klclose()
{
	wflushtty();
}

void
klxint()
{
	register struct tty *tp;

	tp = kl11;
	ttstart();
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}

void
klrint()
{
	register int c;

	c = KLADDR->klrbuf;
	KLADDR->klrcsr |= RDRENB;
	if ((c&0177)==0)
		KLADDR->kltbuf = c;	/* hardware botch */
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
	if(f)
		tp->t_flags = fuword(a);
	else
		suword(a,tp->t_flags);
}

/*
 * general TTY subroutines
 */

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[] = {
	000,000,000,000,004,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,'#',000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	'@',000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

/*
 * The actual structure of a clist block manipulated by
 * getc and putc (mch.s)
 */
struct cblock {
	struct cblock *c_next;
	char info[6];
};

/* The character lists-- space for 6*NCLIST characters */
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

	for (cp=cfree; cp <= &cfree[NCLIST-1]; cp++) {
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
		} else {
			if(maptab[c] && (maptab[c] == c ||(tp->t_flags&LCASE))) {
				if(bp[-2] != '\\')
					c = maptab[c];
				bp--;
			}
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

	tp = kl11;
	if (tp->t_canq.c_cc || canon(tp))
		while (tp->t_canq.c_cc && passc(getc(&tp->t_canq))>=0);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
void
ttwrite()
{
	register struct tty *tp;
	register int c;

	tp = kl11;
	while ((c=cpass())>=0) {
		spl7();
		while (tp->t_outq.c_cc > TTHIWAT) {
			ttstart();
			sleep(&tp->t_outq, TTOPRI);
		}
		spl0();
		ttyoutput(c);
	}
	ttstart();
}

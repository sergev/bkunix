/*
 * TV terminal driver
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
#include "screen.h"

/* base address */
#define KLADDR	0172020	/* console */
#define	NKL11	1
#define DISPPAR	01
#define GRPHMOD	02
#define CRSRENB	04
#define AUTORPT	010

/* special chars */
#define SPERASE	8
#define SPKILL	9
#define SPDISPAR	10
#define SPSTALL	11
#define SPUNSTALL	12
#define SPCLEAR	13


/* special modes */

#define STALLON 2

struct	tty kl11[NKL11];
int klline;
int klcol;
int klscroll;
int dispff;

struct {
	char displist[1][72];
};


struct klregs {
	int klrcsr;
	int klrbuf;
}

klopen()
{
	register struct tty *tp;
	tp = &kl11[0];
	if((tp->t_modes & TOPEN) == 0) {
		tp->t_modes |= TOPEN;
		tp->t_flags = ECHO|CRMOD|LCASE;
		klline = 0;
		klscroll = 0;
		klcol = 0;
		KLADDR->klrcsr |= IENABLE | CRSRENB | AUTORPT;
	}
}

klclose()
{

	flushtty();
}

klrint()
{
	register int c;
	register struct tty *tp;

	tp = &kl11[0];
	c = KLADDR->klrbuf;
	tp->t_modes &= ~STALLON;
	if(c >= 0) klspecial(c >> 8);
	else ttyinput(c);
}

klsgtty(f)
{
	register struct tty *tp;
	register int *a;

	tp = &kl11[0];
	a = u.u_arg[0];
	a += 2;
	flushtty(tp);
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
char	maptab[]
{
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
 * Wait for output to drain, then flush input waiting.
 */
/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register int ccp;
	register struct cblock *cp;

	ccp = cfree;
	for (cp=(ccp+07)&~07; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
}

/*
 * flush all TTY queues
 */
flushtty()
{
	register struct tty *tp;
	register int sps;

	tp = kl11;
	while (getc(&tp->t_canq) >= 0);
	wakeup(&tp->t_rawq);
	sps = spl7();
	while (getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	rstps(sps);
}

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
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
	c = &tp->t_canq;
	while (bp<bp1)
		putc(*bp++, c);
	return(1);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ttyinput(ac)
{
	register int c;
	register struct tty *tp;
	register flags;

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
	}
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
ttyoutput(ac)
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
	klputc(c);

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
		*colp = 0;
	}
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
/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 * The pc is backed up for the duration of this call.
 * In case of a caught interrupt, an RTI will re-execute.
 */
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
ttwrite()
{
	register int c;

	while ((c=cpass())>=0) {
		ttyoutput(c);
	}
}

klputc(c)
char c;
{
	register struct tty *tp;
	register char *cp;

	tp = &kl11[0];
	while(tp->t_modes & STALLON) sleep(tp,TTOPRI);
	switch(c) {

	case '\n':
		klline++;
	case '\r':
		klcol = 0;
		break;
	case 010:
		if((--klcol) < 0) klcol = 0;
		break;
	default:
		if(c < 040) break;
		DISPLIST->displist[klline][klcol++] = c;
		break;
	}
	if(klcol >= NCOL) klcol = NCOL-1;
	if(klline > (NLINE-1)) klline = 0;
	if((klline == klscroll-1) || (klline == NLINE-1 &&
		klscroll == 0)) {
		cp = &DISPLIST->displist[klscroll][0];
		while(cp < &DISPLIST->displist[klscroll][NCOL]) *cp++ = 0;
		klscroll++;
	}
	if(klscroll > (NLINE-1)) klscroll = 0;
	SCNCNTRL->curhcpos = klcol;
	SCNCNTRL->chrloff = klscroll;
	SCNCNTRL->curclnp = (klline >= klscroll) ?
		(klline - klscroll) : (NLINE - klscroll + klline);
}

klspecial(c)
int c;
{

	register char *cp;
	register struct tty *tp;

	tp = &kl11[0];
	switch(c) {

	case SPCLEAR:
		cp = DISPLIST;
		while(cp < SCNCNTRL) *cp++ = 0;
		klline = klscroll = klcol = 0;
		SCNCNTRL->chrloff = 0;
		SCNCNTRL->curhcpos = 0;
		SCNCNTRL->curclnp = 0;
		break;

	case SPSTALL:
		tp->t_modes |= STALLON;
		break;

	case SPUNSTALL:
		tp->t_modes &= ~STALLON;
		wakeup(tp);
		break;

	case SPDISPAR:
		dispff = dispff ? 0 : 1;
		if(dispff) KLADDR->klrcsr |= DISPPAR;
		else KLADDR->klrcsr &= ~DISPPAR;
		break;
	case SPERASE:
		klputc(010);
		klputc(' ');
		klputc(010);
		putc(CERASE,&tp->t_rawq);
		break;

	case SPKILL:
		cp = &DISPLIST->displist[klline][klcol];
		while((--cp) >= &DISPLIST->displist[klline][0]) *cp = 0;
		klcol = 0;
		SCNCNTRL->curhcpos = 0;
		putc(CKILL,&tp->t_rawq);
		break;
	}
}

/*
 * List file or directory
 */
#include <ansidecl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifdef __pdp11__
struct stat {
	int		st_dev;
	int		st_ino;
	int		st_mode;
	char		st_nlink;
	char		st_uid;
	char		st_gid;
	char		st_size0;
	unsigned	st_size;
	int		st_addr[8];
	long		st_atime;
	long		st_mtime;
};
#else
#include <sys/stat.h>
#endif

struct lbuf {
	union {
		char	lname[15];
		char	*namep;
	} ln;
	int	lnum;
	int	lflags;
	char	lnl;
	char	luid;
	char	lgid;
	long	lsize;
	long	lmtime;
};

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, gflg, Iflg;
int	fout;
int	filsys;
int	rflg = 1;
long	year;
int	uidfil = -1;
int	lastuid = -1;
char	tbuf[16];
int	tblocks;
int	statreq;
struct	lbuf	*lastp;
struct	lbuf	*rlastp;
struct	lbuf	*firstp;
char	*dotp = ".";

struct lbuf *gstat PARAMS((char*, int));
void pentry PARAMS((struct lbuf*));
void pmode PARAMS((int));
void readdir PARAMS((char*));
int nblock PARAMS((long));
int compar PARAMS((struct lbuf*, struct lbuf*));

#define devminor(x)	((x) & 0377)
#define devmajor(x)	((x) >> 8 & 0377)

#define	IFMT	060000
#define	DIR	0100000
#define	CHR	020000
#define	BLK	040000
#define	ISARG	01000
#define	LARGE	010000
#define	STXT	010000
#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	RSTXT	01000

int
main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	register struct lbuf *ep, *ep1;
	register struct lbuf *slastp;
	struct lbuf lb;

	firstp = lastp = rlastp = (struct lbuf*) sbrk(0);
	fout = dup(1);
	time(&lb.lmtime);
	year = lb.lmtime - 245L*65536; /* 6 months ago */
	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {

		case 'I':
			if (argc > 1) {
				argc -= 2;
				argv++;
				filsys = open(*argv, 0);
				Iflg++;
				dflg++;
				lflg++;
				statreq++;
			}
			goto out;

		case 'a':
			aflg++;
			continue;

		case 's':
			sflg++;
			statreq++;
			continue;

		case 'd':
			dflg++;
			continue;

		case 'g':
			gflg++;
			continue;

		case 'l':
			lflg++;
			statreq++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'i':
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

		default:
			continue;
		}
		argc--;
	}
out:
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}
	if(lflg) {
		uidfil = open(gflg ? "/etc/group" : "/etc/passwd", 0);
	}
	if (argc==0) {
		argc++;
		argv = &dotp - 1;
	}
	for (i=0; i < argc; i++) {
		ep = gstat(*++argv, 1);
		if (ep== 0)
			continue;
		ep->ln.namep = *argv;
		ep->lflags |= ISARG;
	}
	qsort((char*) firstp, lastp - firstp, sizeof *lastp, compar);
	slastp = lastp;
	for (ep = firstp; ep<slastp; ep++) {
		if (((ep->lflags & DIR) && dflg==0) || fflg) {
			if (argc>1)
				printf("\n%s:\n", ep->ln.namep);
			lastp = slastp;
			readdir(ep->ln.namep);
			if (fflg==0)
				qsort((char*) slastp, lastp - slastp,
					sizeof *lastp,compar);
			if (statreq)
				printf("total %d\n", tblocks);
			for (ep1=slastp; ep1<lastp; ep1++)
				pentry(ep1);
		} else
			pentry(ep);
	}
	return(0);
}

void
pentry(ap)
	struct lbuf *ap;
{
	register int t;
	register struct lbuf *p;
	register char *cp;

	p = ap;
	if (p->lnum == -1)
		return;
	if (iflg)
		printf("%5d ", p->lnum);
	if (sflg)
		printf("%4d ", nblock(p->lsize));
	if (lflg) {
		pmode(p->lflags);
		printf("%2d ", p->lnl);
		t = p->luid;
		if(gflg)
			t = p->lgid;
		t &= 0377;
		printf("%-6d", t);
		if (p->lflags & (BLK|CHR))
			printf("%3d,%3d", devmajor((int)p->lsize),
				devminor((int)p->lsize));
		else
			printf("%7ld", p->lsize);
		cp = ctime(&p->lmtime);
		if(p->lmtime < year)
			printf(" %-7.7s %-4.4s ", cp+4, cp+20); else
			printf(" %-12.12s ", cp+4);
	}
	if (p->lflags&ISARG)
		printf("%s\n", p->ln.namep);
	else
		printf("%.14s\n", p->ln.lname);
}

int
nblock(size)
	long size;
{
	register int n;

	n = size / 512;
	if (size & 0777)
		n++;
	if (n > 8)
		n += (n+255)/256;
	return(n);
}

int	m0[] = { 3, DIR, 'd', BLK, 'b', CHR, 'c', '-'};
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9};

void
pmode(flags)
	int flags;
{
	register int n, *ap, **mp;
	char c;

	for (mp = &m[0]; mp < &m[10];) {
		ap = *mp++;
		n = *ap++;
		while (--n >= 0 && (flags & *ap++) == 0)
			ap++;
		c = *ap;
		write (1, &c, 1);
	}
}

char *
makename(dir, file)
	char *dir, *file;
{
	static char dfile[100];
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	*dp++ = '/';
	fp = file;
	for (i=0; i<14; i++)
		*dp++ = *fp++;
	*dp = 0;
	return(dfile);
}

void
readdir(dir)
	char *dir;
{
	static struct {
		int	dinode;
		char	dname[14];
	} dentry;
	register int j;
	register struct lbuf *ep;
	int fdes;

	fdes = open (dir, 0);
	if (fdes < 0) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		if (read(fdes, (char*) &dentry, 16) < 0) {
			printf("%s read error\n", dir);
			break;
		}
		if (dentry.dinode==0 ||
		    (aflg==0 && dentry.dname[0]=='.'))
			continue;
		if (dentry.dinode == -1)
			break;
		ep = gstat(makename(dir, dentry.dname), 0);
		if (ep->lnum != -1)
			ep->lnum = dentry.dinode;
		for (j=0; j<14; j++)
			ep->ln.lname[j] = dentry.dname[j];
	}
	close(fdes);
}

struct lbuf *
gstat(file, argfl)
	char *file;
{
	struct stat statb;
	register struct lbuf *rep;

	if (lastp+1 >= rlastp) {
		sbrk(512);
		rlastp = (struct lbuf*) ((char*) rlastp + 512);
	}
	rep = lastp;
	lastp++;
	rep->lflags = 0;
	rep->lnum = 0;
	if (argfl || statreq) {
#ifdef __pdp11__
		if (Iflg) {
			register int ino;
			ino = atoi(file);
			seek(filsys, (ino+31)/16, 3);
			seek(filsys, 32*((ino+31)%16), 1);
			read(filsys, (char*) &statb.st_mode, sizeof(statb)-2*sizeof(0));
		} else
#endif
		if (stat(file, (int*) &statb) < 0) {
			printf("%s not found\n", file);
			statb.st_ino = -1;
#ifdef __pdp11__
			statb.st_size0 = 0;
#endif
			statb.st_size = 0;
			statb.st_mode = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
		rep->lnum = statb.st_ino;
		statb.st_mode &= ~DIR;
		if ((statb.st_mode & IFMT) == 060000) {
			statb.st_mode &= ~020000;
		} else if ((statb.st_mode & IFMT) == 040000) {
			statb.st_mode &= ~IFMT;
			statb.st_mode |= DIR;
		}
		statb.st_mode &= ~ LARGE;
		if (statb.st_mode & RSTXT)
			statb.st_mode |= STXT;
		statb.st_mode &= ~ RSTXT;
		rep->lflags = statb.st_mode;
		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
		if (rep->lflags & (BLK|CHR) && lflg)
#ifdef __pdp11__
			rep->lsize = statb.st_addr[0];
#else
			rep->lsize = statb.st_size;
#endif
		else {
			rep->lsize = statb.st_size;
#ifdef __pdp11__
			rep->lsize |= (long) (unsigned char)
				statb.st_size0 << 16;
#endif
			tblocks += nblock(rep->lsize);
		}
		if (uflg) {
			rep->lmtime = statb.st_atime;
		} else {
			rep->lmtime = statb.st_mtime;
		}
	}
	return(rep);
}

int
compar(ap1, ap2)
	struct lbuf *ap1, *ap2;
{
	register struct lbuf *p1, *p2;
	register int i;

	p1 = ap1;
	p2 = ap2;
	if (dflg==0) {
		if ((p1->lflags&(DIR|ISARG)) == (DIR|ISARG)) {
			if ((p2->lflags&(DIR|ISARG)) != (DIR|ISARG))
				return(1);
		} else {
			if ((p2->lflags&(DIR|ISARG)) == (DIR|ISARG))
				return(-1);
		}
	}
	if (tflg) {
		i = 0;
		if (p2->lmtime > p1->lmtime)
			i++;
		else if (p2->lmtime < p1->lmtime)
			i--;
		return(i*rflg);
	}
	return rflg * strcmp((p1->lflags & ISARG) ? p1->ln.namep : p1->ln.lname,
		(p2->lflags & ISARG) ? p2->ln.namep : p2->ln.lname);
}

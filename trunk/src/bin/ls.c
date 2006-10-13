/*
 * List file or directory.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <ansidecl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

struct lbuf {
	union {
		char	lname[15];
		char	*namep;
	} ln;
	int	lnum;
	int	lflags;
	char	lnl;
	char	luid;
	long	lsize;
	long	lmtime;
};

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, Iflg;
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
void dirscan PARAMS((char*));
int nblock PARAMS((long));
int compar PARAMS((const void*, const void*));

#define devminor(x)	((x) & 0377)
#define devmajor(x)	((x) >> 8 & 0377)

#ifdef S_LARGE
#define	S_ISARG		S_LARGE
#else
#define	S_ISARG		S_ISVTX
#endif

int
main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	register struct lbuf *ep, *ep1;
	register int slast;
	struct lbuf lb;

	firstp = lastp = rlastp = (struct lbuf*) malloc(1);
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
		uidfil = open("/etc/passwd", 0);
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
		ep->lflags |= S_ISARG;
	}
	qsort((char*) firstp, lastp - firstp, sizeof *lastp, compar);
	slast = lastp - firstp;
	for (i=0; i<slast; i++) {
		if ((S_ISDIR(firstp[i].lflags) && dflg==0) || fflg) {
			if (argc > 1)
				printf("\n%s:\n", firstp[i].ln.namep);
			lastp = firstp + slast;
			dirscan(firstp[i].ln.namep);
			if (fflg==0)
				qsort((char*) (firstp + slast),
					(lastp - firstp) - slast,
					sizeof *lastp, compar);
			if (statreq)
				printf("total %d\n", tblocks);
			for (ep1=firstp+slast; ep1<lastp; ep1++)
				pentry(ep1);
		} else
			pentry(firstp + i);
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
		t &= 0377;
		printf("%-6d", t);
		if (S_ISBLK(p->lflags) || S_ISCHR(p->lflags))
			printf("%3d,%3d", devmajor((int)p->lsize),
				devminor((int)p->lsize));
		else
			printf("%7ld", p->lsize);
		cp = ctime(&p->lmtime);
		if(p->lmtime < year)
			printf(" %-7.7s %-4.4s ", cp+4, cp+20); else
			printf(" %-12.12s ", cp+4);
	}
	if (p->lflags & S_ISARG)
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
		n += (n + 255) / 256;
	return n;
}

int	m1[] = { 1, 0400, 'r', '-' };
int	m2[] = { 1, 0200, 'w', '-' };
int	m3[] = { 2, S_ISUID, 's', 0100, 'x', '-' };
int	m4[] = { 1, 040, 'r', '-' };
int	m5[] = { 1, 020, 'w', '-' };
int	m6[] = { 2, S_ISGID, 's', 010, 'x', '-' };
int	m7[] = { 1, 4, 'r', '-' };
int	m8[] = { 1, 2, 'w', '-' };
int	m9[] = { 2, S_ISVTX, 't', 1, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9, 0 };

void
pmode(flags)
	int flags;
{
	register int n, *ap, **mp;
	char c;

	c = '-';
	switch (flags & S_IFMT) {
	case S_IFDIR: c = 'd'; break;
	case S_IFCHR: c = 'c'; break;
	case S_IFBLK: c = 'b'; break;
	}
	write (1, &c, 1);
	for (mp = &m[0]; *mp; mp++) {
		ap = *mp;
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
dirscan(dir)
	char *dir;
{
	struct dirent *dentry;
	register struct lbuf *ep;
	DIR *fdes;

	fdes = opendir (dir);
	if (! fdes) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		dentry = readdir(fdes);
		if (! dentry)
			break;
		if (dentry->d_ino == 0 ||
		    (aflg == 0 && dentry->d_name[0]=='.'))
			continue;
/*printf ("<%d %.14s>\n", (int) dentry->d_ino, dentry->d_name);*/
		ep = gstat(makename(dir, dentry->d_name), 0);
/*printf ("- at %p\n", ep);*/
		if (ep->lnum != -1)
			ep->lnum = dentry->d_ino;
		strncpy (ep->ln.lname, dentry->d_name, 14);
		ep->ln.lname[14] = 0;
	}
	closedir(fdes);
}

struct lbuf *
gstat(file, argfl)
	char *file;
{
	struct stat statb;
	register struct lbuf *rep;

	if (lastp+1 >= rlastp) {
		unsigned n;
		n = (rlastp - firstp) + 16;
/*printf ("realloc %p size %d\n", firstp, n * sizeof(*firstp));*/
		rep = firstp;
		firstp = (struct lbuf*) realloc(firstp,	n * sizeof(*firstp));
		rlastp = firstp + n;
		lastp = firstp + (lastp - rep);
/*printf ("        %p - %p\n", firstp, rlastp);*/
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
		if (stat(file, &statb) < 0) {
			printf("%s not found\n", file);
			statb.st_ino = -1;
			statb.st_size = 0;
			statb.st_mode = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
		rep->lnum = statb.st_ino;
		statb.st_mode &= ~ S_ISARG;
		rep->lflags = statb.st_mode;
		rep->luid = statb.st_uid;
		rep->lnl = statb.st_nlink;
		if ((S_ISBLK(rep->lflags) || S_ISCHR(rep->lflags)) && lflg)
#ifdef __pdp11__
			rep->lsize = statb.st_addr[0];
#else
			rep->lsize = statb.st_size;
#endif
		else {
			rep->lsize = statb.st_size;
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
	const void *ap1, *ap2;
{
	register struct lbuf *p1, *p2;
	register int i;

	p1 = (struct lbuf*) ap1;
	p2 = (struct lbuf*) ap2;
	if (dflg==0) {
		if (S_ISDIR(p1->lflags) && (p1->lflags & S_ISARG)) {
			if (! S_ISDIR(p2->lflags) || ! (p2->lflags & S_ISARG))
				return(1);
		} else {
			if (S_ISDIR(p2->lflags) && (p2->lflags & S_ISARG))
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
	return rflg * strcmp((p1->lflags & S_ISARG) ? p1->ln.namep : p1->ln.lname,
		(p2->lflags & S_ISARG) ? p2->ln.namep : p2->ln.lname);
}

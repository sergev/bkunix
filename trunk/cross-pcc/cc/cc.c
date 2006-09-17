/*
 * cc - front end for C compiler
 */
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/dir.h>
#include <sys/wait.h>

char	*cpp = "/usr/bin/cpp";
char	*ccom = "/usr/local/lib/pdp11/c0";
char	*ccom1 = "/usr/local/lib/pdp11/c1";
char	*c2 = "/usr/local/lib/pdp11/c2";
char	*as = "/usr/local/bin/pdp11-as";
char	*ld = "/usr/local/bin/pdp11-ld";
char	*crt0 = "/usr/local/lib/pdp11/crt0.o";

char	tmp0[30];		/* big enough for /tmp/ctm%05.5d */
char	*tmp1, *tmp2, *tmp3, *tmp4, *tmp5;
char	*outfile;
char	*savestr(), *strspl(), *setsuf();
char	**av, **clist, **llist, **plist;
int	cflag, eflag, oflag, pflag, sflag, wflag, exflag, proflag;
int	Mflag, vflag;
int	exfail;
char	*chpass;
char	*npassname;

int	nc, nl, np, nxo, na;

#define	cunlink(s)	if (s) unlink(s)

void error(s, x)
	char *s, *x;
{
	FILE *diag = exflag ? stderr : stdout;

	fprintf(diag, "cc: ");
	fprintf(diag, s, x);
	putc('\n', diag);
	exfail++;
	cflag++;
	eflag++;
}

int getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while ((t = *s++) != 0)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c <= MAXNAMLEN && c > 2 && *s++ == '.')
		return (*s);
	return (0);
}

char *
setsuf(as, ch)
	char *as;
{
	register char *s, *s1;

	s = s1 = savestr(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return (s1);
}

int inlist(l, os)
	char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	while ((t = *l++) != 0) {
		while ((c = *s++) != 0)
			if (c != *t++)
				break;
		if (*t==0 && c==0)
			return (1);
		s = os;
	}
	return (0);
}

void dexit()
{
	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
	}
	exit(eflag);
}

void idexit()
{
	eflag = 100;
	dexit();
}

int callsys(f, v)
	char *f, **v;
{
	int t, status;
	char **cpp;

	if (vflag) {
		printf("  %s", f);
		for (cpp = v+1; *cpp != 0; cpp++)
			printf(" %s", *cpp);
		printf("\n");
	}
	t = vfork();
	if (t == -1) {
		printf("No more processes\n");
		return (100);
	}
	if (t == 0) {
		execv(f, v);
		printf("Can't find %s\n", f);
		fflush(stdout);
		_exit(100);
	}
	while (t != wait(&status))
		;
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return ((status>>8) & 0377);
}

int main(argc, argv)
	char **argv;
{
	char *t;
	char *assource;
	int i, j, c;

	/* ld currently adds upto 5 args; 10 is room to spare */
	av = (char **)calloc(argc+10, sizeof (char **));
	clist = (char **)calloc(argc, sizeof (char **));
	llist = (char **)calloc(argc, sizeof (char **));
	plist = (char **)calloc(argc, sizeof (char **));
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') switch (argv[i][1]) {

		case 'S':
			sflag++;
			cflag++;
			continue;
		case 'o':
			if (++i < argc) {
				outfile = argv[i];
				switch (getsuf(outfile)) {

				case 'c':
					error("-o would overwrite %s",
					    outfile);
					exit(8);
				}
			}
			continue;
		case 'O':
			oflag++;
			continue;
		case 'p':
			proflag++;
			crt0 = "/usr/local/lib/pdp11/mcrt0.o";
			if (argv[i][2] == 'g')
				crt0 = "/usr/local/lib/pdp11/gcrt0.o";
			continue;
		case 'w':
			wflag++;
			continue;
		case 'v':
			vflag++;
			continue;
		case 'E':
			exflag++;
		case 'P':
			pflag++;
			if (argv[i][1]=='P')
				fprintf(stderr,
	"cc: warning: -P option obsolete; you should use -E instead\n");
			plist[np++] = argv[i];
		case 'c':
			cflag++;
			continue;
		case 'M':
			exflag++;
			pflag++;
			Mflag++;
			/* and fall through */
		case 'D':
		case 'I':
		case 'U':
		case 'C':
			plist[np++] = argv[i];
			continue;
		case 'L':
			llist[nl++] = argv[i];
			continue;
		case 't':
			if (chpass)
				error("-t overwrites earlier option", 0);
			chpass = argv[i]+2;
			if (chpass[0]==0)
				chpass = "012p";
			continue;
		case 'B':
			if (npassname)
				error("-B overwrites earlier option", 0);
			npassname = argv[i]+2;
			if (npassname[0]==0)
				npassname = "/usr/c/o";
			continue;
		}
		t = argv[i];
		c = getsuf(t);
		if (c=='c' || c=='s' || exflag) {
			clist[nc++] = t;
			t = setsuf(t, 'o');
		}
		if (getsuf(t) != 'o' || ! inlist(llist, t)) {
			llist[nl++] = t;
			if (getsuf(t)=='o')
				nxo++;
		}
	}
	if (npassname && chpass ==0)
		chpass = "012p";
	if (chpass && npassname==0)
		npassname = "/usr/new";
	if (chpass)
	for (t=chpass; *t; t++) {
		switch (*t) {

		case '0':
			ccom = strspl(npassname, "c0");
			continue;
		case '1':
			ccom1 = strspl(npassname, "c1");
			continue;
		case '2':
			c2 = strspl(npassname, "c2");
			continue;
		case 'p':
			cpp = strspl(npassname, "cpp");
			continue;
		}
	}
	if (nc==0)
		goto nocom;
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, idexit);
	if (pflag==0)
		sprintf(tmp0, "/tmp/ctm%05d", getpid());
	tmp1 = strspl(tmp0, "1");
	tmp2 = strspl(tmp0, "2");
	tmp3 = strspl(tmp0, "3");
	if (pflag==0)
		tmp4 = strspl(tmp0, "4");
	if (oflag)
		tmp5 = strspl(tmp0, "5");
	for (i=0; i<nc; i++) {
		if (nc > 1 && !Mflag) {
			printf("%s:\n", clist[i]);
			fflush(stdout);
		}
		if (!Mflag && getsuf(clist[i]) == 's') {
			assource = clist[i];
			goto assemble;
		} else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		av[0] = "cpp"; av[1] = clist[i];
		na = 2;
		if (!exflag)
			av[na++] = tmp4;
		for (j = 0; j < np; j++)
			av[na++] = plist[j];
		av[na++] = 0;
		if (callsys(cpp, av)) {
			exfail++;
			eflag++;
		}
		if (pflag || exfail) {
			cflag++;
			continue;
		}
		if (sflag) {
			if (nc==1 && outfile)
				tmp3 = outfile;
			else
				tmp3 = setsuf(clist[i], 's');
			assource = tmp3;
		}
		av[0] = "c0";
		av[1] = tmp4; av[2] = tmp1; av[3] = tmp2; na = 4;
		if (proflag)
			av[na++] = "-P";
		if (wflag)
			av[na++] = "-w";
		av[na] = 0;
		if (callsys(ccom, av)) {
			cflag++;
			eflag++;
			continue;
		}
		av[0] = "c1"; av[1] = tmp1; av[2] = tmp2;
		av[3] = oflag ? tmp5 : tmp3;
		av[4] = 0;
		if (callsys(ccom1, av)) {
			cflag++;
			eflag++;
			continue;
		}
		if (oflag) {
			av[0] = "c2"; av[1] = tmp5; av[2] = tmp3; av[3] = 0;
			if (callsys(c2, av)) {
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} else
				unlink(tmp5);
		}
		if (sflag)
			continue;
assemble:
		cunlink(tmp1); cunlink(tmp2); cunlink(tmp4);
		av[0] = "as"; av[1] = "-V";
		av[2] = "-u"; av[3] = "-o";
		if (cflag && nc==1 && outfile)
			av[4] = outfile;
		else
			av[4] = setsuf(clist[i], 'o');
		av[5] = assource;
		av[6] = 0;
		if (callsys(as, av) > 1) {
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
		av[0] = "ld"; av[1] = "-X"; av[2] = crt0; na = 3;
		if (outfile) {
			av[na++] = "-o";
			av[na++] = outfile;
		}
		while (i < nl)
			av[na++] = llist[i++];
		if (proflag)
			av[na++] = "-lc_p";
		else
			av[na++] = "-lc";
		av[na++] = 0;
		eflag |= callsys(ld, av);
		if (nc==1 && nxo==1 && eflag==0)
			unlink(setsuf(clist[0], 'o'));
	}
	dexit();
	return (0);
}

#define	NSAVETAB	1024
char	*savetab;
int	saveleft;

char *
savestr(cp)
	register char *cp;
{
	register int len;

	len = strlen(cp) + 1;
	if (len > saveleft) {
		saveleft = NSAVETAB;
		if (len > saveleft)
			saveleft = len;
		savetab = (char *)malloc(saveleft);
		if (savetab == 0) {
			fprintf(stderr, "ran out of memory (savestr)\n");
			exit(1);
		}
	}
	strncpy(savetab, cp, len);
	cp = savetab;
	savetab += len;
	saveleft -= len;
	return (cp);
}

char *
strspl(left, right)
	char *left, *right;
{
	char buf[BUFSIZ];

	strcpy(buf, left);
	strcat(buf, right);
	return (savestr(buf));
}

/*
 * cc - front end for Rithie's C compiler
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

char	*cpp = DESTDIR "/lib/pdp11/cpp";
char   *ccom = DESTDIR "/lib/pdp11/c0";
char   *ccom1 = DESTDIR "/lib/pdp11/c1";
char   *c2 = DESTDIR "/lib/pdp11/c2";
char   *as = DESTDIR "/bin/pdp11-asm";
char	*ld = DESTDIR "/bin/pdp11-ld";
char	*crt0 = DESTDIR "/lib/pdp11/crt0.o";

char	tmp0[30];		/* big enough for /tmp/ctm%05.5d */
char	*tmp_c1, *tmp_c2, *tmp_asm, *tmp_pre, *tmp_opt;
char	*outfile;
char	*savestr(), *strspl(), *setsuf();
char	**av, **clist, **llist, **plist;
int	cflag, Oflag, Pflag, Sflag, Eflag, proflag, vflag, wflag, Lminusflag;
int	errflag;
int	exfail;

int	nc, nl, np, nxo, na;

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

void cleanup()
{
	if (! Pflag) {
		if (tmp_c1)
			unlink(tmp_c1);
		if (tmp_c2)
			unlink(tmp_c2);
		if (Sflag==0 && tmp_asm)
			unlink(tmp_asm);
		if (tmp_pre)
			unlink(tmp_pre);
		if (tmp_opt)
			unlink(tmp_opt);
	}
}

void killed()
{
	cleanup();
	exit(100);
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
		cleanup();
		if (t!=2) {
			printf("Fatal error in %s\n", f);
			exit(8);
		}
		exit(errflag);
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
		if (*argv[i] == '-') {
			switch (argv[i][1]) {
			case 'S':
				Sflag++;
				cflag++;
				continue;
			case 'o':
				if (++i < argc) {
					outfile = argv[i];
					switch (getsuf(outfile)) {
					case 'c':
						fprintf(stderr, "cc: -o would overwrite %s\n",
						    outfile);
						exit(8);
					}
				}
				continue;
			case 'O':
				Oflag++;
				continue;
			case 'p':
				proflag++;
				crt0 = DESTDIR "/lib/pdp11/mcrt0.o";
				if (argv[i][2] == 'g')
					crt0 = DESTDIR "/lib/pdp11/gcrt0.o";
				continue;
			case 'w':
				wflag++;
				continue;
			case 'v':
				vflag++;
				continue;
			case 'E':
				Eflag++;
			case 'P':
				Pflag++;
				plist[np++] = argv[i];
			case 'c':
				cflag++;
				continue;
			case 'D':
			case 'I':
			case 'U':
			case 'C':
				plist[np++] = argv[i];
				continue;
			case 'L':
				if (argv[i][2] == '-')
					Lminusflag++;
				else
					llist[nl++] = argv[i];
				continue;
			}
		}
		t = argv[i];
		c = getsuf(t);
		if (c=='c' || c=='s' || c=='S' || Eflag) {
			clist[nc++] = t;
			t = setsuf(t, 'o');
		}
		if (getsuf(t) != 'o' || ! inlist(llist, t)) {
			llist[nl++] = t;
			if (getsuf(t)=='o')
				nxo++;
		}
	}
	if (nc==0)
		goto nocom;
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, killed);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, killed);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, killed);
	sprintf(tmp0, "/tmp/ctm%05d", getpid());
	tmp_c1 = strspl(tmp0, "1");
	tmp_c2 = strspl(tmp0, "2");
	tmp_asm = strspl(tmp0, "3");
	if (! Pflag)
		tmp_pre = strspl(tmp0, "4");
	if (Oflag)
		tmp_opt = strspl(tmp0, "5");
	for (i=0; i<nc; i++) {
		if (nc > 1) {
			printf("%s:\n", clist[i]);
			fflush(stdout);
		}
		if (getsuf(clist[i]) == 's') {
			assource = clist[i];
			goto assemble;
		}
		if (Sflag) {
			if (nc==1 && outfile)
				tmp_asm = outfile;
			else
				tmp_asm = setsuf(clist[i], 's');
		}
		assource = tmp_asm;
		if (Pflag)
			tmp_pre = setsuf(clist[i], 'i');

		/* Preprocessor. */
		av[0] = "cpp";
		na = 1;
		/* Ritchie's cc does not support ansi function declarations. */
		av[na++] = "-D__pdp11__=1";
		for (j = 0; j < np; j++)
			av[na++] = plist[j];
		if (! Eflag) {
			av[na++] = "-o";
			if (getsuf(clist[i]) == 'S' && ! Pflag)
				av[na++] = tmp_asm;
			else
				av[na++] = tmp_pre;
		}
		av[na++] = clist[i];
		av[na] = 0;
		if (callsys(cpp, av)) {
			exfail++;
			errflag++;
		}
		if (Pflag || exfail) {
			cflag++;
			continue;
		}
		if (getsuf(clist[i]) == 'S')
			goto assemble;

		/* Compiler pass 1. */
		av[0] = "c0";
		na = 1;
		av[na++] = tmp_pre;
		av[na++] = tmp_c1;
		av[na++] = tmp_c2;
		if (proflag)
			av[na++] = "-P";
		if (wflag)
			av[na++] = "-w";
		av[na] = 0;
		if (callsys(ccom, av)) {
			cflag++;
			errflag++;
			continue;
		}

		/* Compiler pass 2. */
		av[0] = "c1";
		na = 1;
		av[na++] = tmp_c1;
		av[na++] = tmp_c2;
		av[na++] = Oflag ? tmp_opt : tmp_asm;
		av[na] = 0;
		if (callsys(ccom1, av)) {
			cflag++;
			errflag++;
			continue;
		}

		/* Optimizer. */
		if (Oflag) {
			av[0] = "c2";
			na = 1;
			av[na++] = tmp_opt;
			av[na++] = tmp_asm;
			av[na] = 0;
			if (callsys(c2, av)) {
				unlink(tmp_asm);
				tmp_asm = assource = tmp_opt;
			} else
				unlink(tmp_opt);
		}
		if (Sflag)
			continue;
assemble:
		if (tmp_pre)
			unlink(tmp_pre);
		if (tmp_c1)
			unlink(tmp_c1);
		if (tmp_c2)
			unlink(tmp_c2);

		/* Assembler. */
		av[0] = "as";
		na = 1;
		av[na++] = "-u";
		av[na++] = "-o";
		if (cflag && nc==1 && outfile)
			av[na++] = outfile;
		else
			av[na++] = setsuf(clist[i], 'o');
		av[na++] = assource;
		av[na] = 0;
		if (callsys(as, av) > 1) {
			cflag++;
			errflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;

		/* Linker. */
		av[0] = "ld";
		na = 1;
		av[na++] = "-X";
		if (outfile) {
			av[na++] = "-o";
			av[na++] = outfile;
		}
		av[na++] = crt0;
		while (i < nl)
			av[na++] = llist[i++];
		if (! Lminusflag) {
			av[na++] = "-L" DESTDIR "/lib/pdp11";
			av[na++] = proflag ? "-lc_p" : "-lc";
			av[na++] = proflag ? "-lcrt_p" : "-lcrt";
		}
		av[na++] = 0;
		errflag |= callsys(ld, av);
		if (nc==1 && nxo==1 && errflag==0)
			unlink(setsuf(clist[0], 'o'));
	}
	cleanup();
	return (errflag);
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

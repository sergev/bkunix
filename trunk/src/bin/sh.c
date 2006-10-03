/*
 * Modified for fake on-disk 'pipes'
 */
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>

#define LINSIZ	1000
#define ARGSIZ	50
#define TRESIZ	100

#define QUOTE	0200
#define FAND	1
#define FCAT	2
#define FPIN	4
#define FPOU	8
#define FPAR	16
#define FINT	32
#define FPRS	64
#define TCOM	1
#define TPAR	2
#define TFIL	3
#define TLST	4
#define DTYP	0
#define DLEF	1
#define DRIT	2
#define DFLG	3
#define DSPR	4
#define DCOM	5
#define	ENOMEM	12
#define	ENOEXEC	8

char	*dolp;
char	pidp[6];
char	**dolv;
int	dolc;
char	*promp;
char	*linep;
char	*elinep;
char	**argp;
char	**eargp;
int	*treep;
int	*treeend;
char	peekc;
char	gflg;
char	error;
char	uid;
char	setintr;
char	*arginp;
int	onelflg;
char	*pipe = ".__pf";
jmp_buf	cmdloop;

char	*mesg[] = {
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	0,
	"Sig 14",
	"Sig 15",
	"Sig 16",
	"Sig 17",
	"Sig 18",
	"Sig 19",
};

char	line[LINSIZ];
char	*args[ARGSIZ];
int	trebuf[TRESIZ];

extern int errno;

void err PARAMS((char*));
void prs PARAMS((char*));
int getc PARAMS((void));
int readc PARAMS((void));
void main1 PARAMS((void));
void word PARAMS((void));
int *syntax PARAMS((char**, char**));
void execute PARAMS((int*, int*, int*));
int any PARAMS((int, char*));
int *syn1 PARAMS((char**, char**));
int *syn2 PARAMS((char**, char**));
int *syn3 PARAMS((char**, char**));
int equal PARAMS((char*, char*));
void pwait PARAMS((int, int*));
void prn PARAMS((int));
void texec PARAMS((char*, int*));
void putc PARAMS((char));

int
main(c, av)
	int c;
	char **av;
{
	register int f;
	register char **v;

	for (f=3; f<15; f++)
		close(f);
	dolc = getpid();
	for (f=4; f>=0; f--) {
		pidp[f] = dolc % 10 + '0';
		dolc /= 10;
	}
	v = av;
	promp = "% ";
	uid = getuid();
	if ((uid & 0377) == 0) {
		promp = "# ";
	}
	if (c > 1) {
		promp = 0;
		if (*v[1]=='-') {
			**v = '-';
			if (v[1][1]=='c' && c>2)
				arginp = v[2];
			else if (v[1][1]=='t')
				onelflg = 2;
		} else {
			close(0);
			f = open(v[1], 0);
			if (f < 0) {
				prs(v[1]);
				err(": cannot open");
			}
		}
	}
	if (**v == '-') {
		setintr++;
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
	}
	dolv = v+1;
	dolc = c-1;

loop:
	if (promp != 0)
		prs(promp);
	peekc = getc();
	main1();
	goto loop;
}

void
main1()
{
	register char *cp;
	register int *t;

	argp = args;
	eargp = args+ARGSIZ-5;
	linep = line;
	elinep = line+LINSIZ-5;
	error = 0;
	gflg = 0;
	do {
		cp = linep;
		word();
	} while (*cp != '\n');
	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	if (gflg == 0) {
		if (error == 0) {
			setjmp(cmdloop);
			if (error)
				return;
			t = syntax(args, argp);
		}
		if (error != 0)
			err("syntax error");
		else
			execute(t, 0, 0);
	}
}

void
word()
{
	register char c, c1;

	*argp++ = linep;

loop:
	switch (c = getc()) {

	case ' ':
	case '\t':
		goto loop;

	case '\'':
	case '"':
		c1 = c;
		while ((c=readc()) != c1) {
			if (c == '\n') {
				error++;
				peekc = c;
				return;
			}
			*linep++ = c|QUOTE;
		}
		goto pack;

	case '&':
	case ';':
	case '<':
	case '>':
	case '(':
	case ')':
	case '|':
	case '^':
	case '\n':
		*linep++ = c;
		*linep++ = '\0';
		return;
	}

	peekc = c;

pack:
	for (;;) {
		c = getc();
		if (any(c, " '\"\t;&<>()|^\n")) {
			peekc = c;
			if (any(c, "\"'"))
				goto loop;
			*linep++ = '\0';
			return;
		}
		*linep++ = c;
	}
}

int *
tree(n)
	int n;
{
	register int *t;

	t = treep;
	treep += n;
	if (treep>treeend) {
		prs("Command line overflow\n");
		error++;
		longjmp(cmdloop, 1);
	}
	return t;
}

int
getc()
{
	register char c;

	if (peekc) {
		c = peekc;
		peekc = 0;
		return c;
	}
	if (argp > eargp) {
		argp -= 10;
		while ((c=getc()) != '\n');
		argp += 10;
		err("Too many args");
		gflg++;
		return c;
	}
	if (linep > elinep) {
		linep -= 10;
		while ((c=getc()) != '\n');
		linep += 10;
		err("Too many characters");
		gflg++;
		return c;
	}
getd:
	if (dolp) {
		c = *dolp++;
		if (c != '\0')
			return c;
		dolp = 0;
	}
	c = readc();
	if (c == '\\') {
		c = readc();
		if (c == '\n')
			return ' ';
		return c | QUOTE;
	}
	if (c == '$') {
		c = readc();
		if (c>='0' && c<='9') {
			if (c-'0' < dolc)
				dolp = dolv[c-'0'];
			goto getd;
		}
		if (c == '$') {
			dolp = pidp;
			goto getd;
		}
	}
	return c & 0177;
}

int
readc()
{
	char cc;
	register int c;

	if (arginp) {
		if (arginp == &error)
			exit(1);
		if ((c = *arginp++) == 0) {
			arginp = &error;
			c = '\n';
		}
		return c;
	}
	if (onelflg==1)
		exit(1);
	if (read(0, &cc, 1) != 1) {
		if (!promp) exit(1);
		else error++;
		return 0;
	}
	if (cc=='\n' && onelflg)
		onelflg--;
	return cc;
}

/*
 * syntax
 *	empty
 *	syn1
 */
int *
syntax(p1, p2)
	char **p1, **p2;
{

	while (p1 != p2) {
		if (any(**p1, ";&\n"))
			p1++; else
			return syn1(p1, p2);
	}
	return 0;
}

/*
 * syn1
 *	syn2
 *	syn2 & syntax
 *	syn2 ; syntax
 */
int *
syn1(p1, p2)
	char **p1, **p2;
{
	register char **p;
	register int *t, *t1;
	int l;

	l = 0;
	for (p=p1; p!=p2; p++)
	switch (**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		if (l < 0)
			error++;
		continue;

	case '&':
	case ';':
	case '\n':
		if (l == 0) {
			l = **p;
			t = tree(4);
			t[DTYP] = TLST;
			t[DLEF] = (int) syn2(p1, p);
			t[DFLG] = 0;
			if (l == '&') {
				t1 = (int*) t[DLEF];
				t1[DFLG] |= FAND|FPRS|FINT;
			}
			t[DRIT] = (int) syntax(p+1, p2);
			return t;
		}
	}
	if (l == 0)
		return syn2(p1, p2);
	error++;
	return 0;
}

/*
 * syn2
 *	syn3
 *	syn3 | syn2
 */
int *
syn2(p1, p2)
	char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for (p=p1; p!=p2; p++)
	switch (**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
	case '^':
		if (l == 0) {
			t = tree(4);
			t[DTYP] = TFIL;
			t[DLEF] = (int) syn3(p1, p);
			t[DRIT] = (int) syn2(p+1, p2);
			t[DFLG] = 0;
			return t;
		}
	}
	return syn3(p1, p2);
}

/*
 * syn3
 *	( syn1 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 */
int *
syn3(p1, p2)
	char **p1, **p2;
{
	register char **p;
	char **lp, **rp;
	register int *t;
	int n, l, i, o, c, flg;

	flg = 0;
	if (**p2 == ')')
		flg |= FPAR;
	lp = 0;
	rp = 0;
	i = 0;
	o = 0;
	n = 0;
	l = 0;
	for (p=p1; p!=p2; p++)
	switch (c = **p) {

	case '(':
		if (l == 0) {
			if (lp != 0)
				error++;
			lp = p+1;
		}
		l++;
		continue;

	case ')':
		l--;
		if (l == 0)
			rp = p;
		continue;

	case '>':
		p++;
		if (p!=p2 && **p=='>')
			flg |= FCAT; else
			p--;

	case '<':
		if (l == 0) {
			p++;
			if (p == p2) {
				error++;
				p--;
			}
			if (any(**p, "<>("))
				error++;
			if (c == '<') {
				if (i != 0)
					error++;
				i = (int) *p;
				continue;
			}
			if (o != 0)
				error++;
			o = (int) *p;
		}
		continue;

	default:
		if (l == 0)
			p1[n++] = *p;
	}
	if (lp != 0) {
		if (n != 0)
			error++;
		t = tree(5);
		t[DTYP] = TPAR;
		t[DSPR] = (int) syn1(lp, rp);
		goto out;
	}
	if (n == 0)
		error++;
	p1[n++] = 0;
	t = tree(n+5);
	t[DTYP] = TCOM;
	for (l=0; l<n; l++)
		t[l+DCOM] = (int) p1[l];
out:
	t[DFLG] = flg;
	t[DLEF] = i;
	t[DRIT] = o;
	return t;
}

void
scan(at, f)
	int *at;
	int (*f)();
{
	register char *p, c;
	register int *t;

	t = at+DCOM;
	while ((p = (char*) *t++))
		while ((c = *p))
			*p++ = (*f)(c);
}

int
tglob(c)
	int c;
{
	if (any(c, "[?*"))
		gflg = 1;
	return c;
}

int
trim(c)
	int c;
{
	return c & 0177;
}

void
execute(t, pf1, pf2)
	int *t, *pf1, *pf2;
{
	int i, f, pv[2];
	register int *t1;
	register char *cp1, *cp2;

	if (t != 0)
	switch (t[DTYP]) {

	case TCOM:
		cp1 = (char*) t[DCOM];
		if (equal(cp1, "cd")) {
			if (t[DCOM+1] != 0) {
				if (chdir((char*) t[DCOM+1]) < 0)
					err("chdir: bad directory");
			} else
				err("chdir: arg count");
			return;
		}
		if (equal(cp1, "shift")) {
			if (dolc < 1) {
				prs("shift: no args\n");
				return;
			}
			dolv[1] = dolv[0];
			dolv++;
			dolc--;
			return;
		}
		if (equal(cp1,"sync")) {
			sync();
			return;
		}
		if (equal(cp1, "wait")) {
			pwait(-1, 0);
			return;
		}
		if (equal(cp1, ":"))
			return;

	case TPAR:
		f = t[DFLG];
		i = 0;
		if ((f&FPAR) == 0)
			i = fork();
		if (i == -1) {
			err("try again");
			return;
		}
		if (i != 0) {
			if ((f&FPIN) != 0) {
				close(pf1[0]);
				close(pf1[1]);
			}
			if ((f&FPRS) != 0) {
				prn(i);
				prs("\n");
			}
			if ((f&FAND) != 0)
				return;
			pwait(i, t);
			return;
		}
		if (t[DLEF] != 0) {
			close(0);
			i = open((char*) t[DLEF], 0);
			if (i < 0) {
				prs((char*) t[DLEF]);
				err(": cannot open");
				exit(1);
			}
		}
		if (t[DRIT] != 0) {
			if ((f&FCAT) != 0) {
				i = open((char*) t[DRIT], 1);
				if (i >= 0) {
					seek(i, 0, 2);
					goto f1;
				}
			}
			i = creat((char*) t[DRIT], 0666);
			if (i < 0) {
				prs((char*) t[DRIT]);
				err(": cannot create");
				exit(1);
			}
		f1:
			close(1);
			dup(i);
			close(i);
		}
		if ((f&FPIN) != 0) {
			close(0);
			dup(pf1[0]);
			close(pf1[0]);
			close(pf1[1]);
		}
		if ((f&FPOU) != 0) {
			close(1);
			dup(pf2[1]);
			close(pf2[0]);
			close(pf2[1]);
		}
		if ((f&FINT)!=0 && t[DLEF]==0 && (f&FPIN)==0) {
			close(0);
			open("/dev/null", 0);
		}
		if ((f&FINT) == 0 && setintr) {
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
		}
		if (t[DTYP] == TPAR) {
			t1 = (int*) t[DSPR];
			if (t1)
				t1[DFLG] |= f&FINT;
			execute(t1, 0, 0);
			exit(1);
		}
		gflg = 0;
		scan(t, tglob);
		if (gflg) {
			t[DSPR] = (int) "/etc/glob";
			execv((char*) t[DSPR], (char**) (t+DSPR));
			prs("glob: cannot execute\n");
			exit(1);
		}
		scan(t, trim);
		*linep = 0;
		texec((char*) t[DCOM], t);
		cp1 = linep;
		cp2 = "/usr/bin/";
		while ((*cp1 = *cp2++))
			cp1++;
		cp2 = (char*) t[DCOM];
		while ((*cp1++ = *cp2++))
			continue;
		texec(linep+4, t);
/*
		texec(linep, t);
*/
		prs((char*) t[DCOM]);
		err(": not found");
		exit(1);

	case TFIL:
		f = t[DFLG];
/*
 * fake pipe mods follow
 */
		close(creat(pipe,0666));
		pv[0] = open(pipe,0);
		pv[1] = open(pipe,1);
		unlink(pipe);
/*
 * original was just pipe(pv)
 */
		t1 = (int*) t[DLEF];
		t1[DFLG] |= FPOU | (f&(FPIN|FINT|FPRS));
		execute(t1, pf1, pv);
		t1 = (int*) t[DRIT];
		t1[DFLG] |= FPIN | (f&(FPOU|FINT|FAND|FPRS));
		execute(t1, pv, pf2);
		return;

	case TLST:
		f = t[DFLG]&FINT;
		t1 = (int*) t[DLEF];
		if (t1)
			t1[DFLG] |= f;
		execute(t1, 0, 0);
		t1 = (int*) t[DRIT];
		if (t1)
			t1[DFLG] |= f;
		execute(t1, 0, 0);
		return;

	}
}

void
texec(f, at)
	char *f;
	int *at;
{
	register int *t;

	t = at;
	execv(f, (char**) (t+DCOM));
	if (errno==ENOEXEC) {
		if (*linep)
			t[DCOM] = (int) linep;
		t[DSPR] = (int) "/bin/sh";
		execv((char*) t[DSPR], (char**) (t+DSPR));
		prs("No shell!\n");
		exit(1);
	}
	if (errno==ENOMEM) {
		prs((char*) t[DCOM]);
		err(": too large");
		exit(1);
	}
}

void
err(s)
	char *s;
{
	prs(s);
	prs("\n");
	if (promp == 0) {
		seek(0, 0, 2);
		exit(1);
	}
}

void
prs(as)
	char *as;
{
	register char *s;

	s = as;
	while (*s)
		putc(*s++);
}

void
putc(c)
	char c;
{
	write(2, &c, 1);
}

void
prn(n)
	int n;
{
	register int a;

	a = (unsigned) n / 10;
	if (a)
		prn(a);
	putc((unsigned) n % 10 + '0');
}

int
any(c, as)
	int c;
	char *as;
{
	register char *s;

	s = as;
	while (*s)
		if (*s++ == c)
			return 1;
	return 0;
}

int
equal(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return 1;
	return 0;
}

void
pwait(i, t)
	int i, *t;
{
	register int p, e;
	int s;

	if (i != 0)
	for (;;) {
		p = wait(&s);
		if (p == -1)
			break;
		e = s & 0177;
		if (mesg[e] != 0) {
			if (p != i) {
				prn(p);
				prs(": ");
			}
			prs(mesg[e]);
			if (s & 0200)
				prs(" -- Core dumped");
		}
		if (e != 0)
			err("");
		if (i == p) {
			break;
		}
	}
}

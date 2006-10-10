/*
 * Global command --
 *	glob params
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 * Perform command with argument list constructed as follows:
 * - if param does not contain "*", "[", or "?", use it as is
 * - if it does, find all files in current directory
 *   which match the param, sort them, and use them
 *
 * Prepend the command name with "/bin" or "/usr/bin" as required.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#define	E2BIG	7
#define	ENOEXEC	8
#define	ENOENT	2

#define	STRSIZ	522

char	ab[STRSIZ];		/* generated characters */
char	*ava[200];		/* generated arguments */
char	**av = &ava[1];
char	*string = ab;
int	errno;
int	ncoll;

void
toolong()
{
	write(2, "Arg list too long\n", 18);
	exit(1);
}

char *
cat(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;
	register int c;

	s2 = string;
	s1 = as1;
	while ((c = *s1++)) {
		if (s2 > &ab[STRSIZ])
			toolong();
		c &= 0177;
		if (c==0) {
			*s2++ = '/';
			break;
		}
		*s2++ = c;
	}
	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ])
			toolong();
		*s2++ = c = *s1++;
	} while (c);
	s1 = string;
	string = s2;
	return(s1);
}

int
amatch(as, ap)
	char *as, *ap;
{
	register char *s, *p;
	register int scc;
	int c, cc, ok, lc;
	int umatch();

	s = as;
	p = ap;
	scc = *s++;
	if (scc) {
		scc &= 0177;
		if (scc == 0)
			scc = 0200;
	}
	c = *p++;
	switch (c) {

	case '[':
		ok = 0;
		lc = 077777;
		while ((cc = *p++)) {
			if (cc==']') {
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

int
match(s, p)
	char *s, *p;
{
	if (*s=='.' && *p!='.')
		return(0);
	return(amatch(s, p));
}

int
umatch(s, p)
	char *s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}

int
compar(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

void
sort(oav)
	char **oav;
{
	register char **p1, **p2, *c;

	p1 = oav;
	while (p1 < av-1) {
		p2 = p1;
		while(++p2 < av) {
			if (compar(*p1, *p2) > 0) {
				c = *p1;
				*p1 = *p2;
				*p2 = c;
			}
		}
		p1++;
	}
}

void
expand(as)
	char *as;
{
	register char *s, *cs;
	register DIR *dirf;
	char **oav;
	struct dirent *entry;

	s = cs = as;
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) {
			*av++ = cat(s, "");
			return;
		}
	}
	for (;;) {
		if (cs==s) {
			dirf = opendir(".");
			s = "";
			break;
		}
		if (*--cs == '/') {
			*cs = 0;
			dirf = opendir(s==cs ? "/" : s);
			*cs++ = 0200;
			break;
		}
	}
	if (! dirf) {
		write(2, "No directory\n", 13);
		exit(1);
	}
	oav = av;
	while ((entry = readdir(dirf))) {
		if (match(entry->d_name, cs)) {
			*av++ = cat(s, entry->d_name);
			ncoll++;
		}
	}
	closedir(dirf);
	sort(oav);
}

void
execute(afile, aarg)
	char *afile;
	char **aarg;
{
	register char *file, **arg;

	arg = aarg;
	file = afile;
	execv(file, arg);
	if (errno==ENOEXEC) {
		arg[0] = file;
		*--arg = "/bin/sh";
		execv(*arg, arg);
	}
	if (errno==E2BIG)
		toolong();
}

int
main(argc, argv)
	char **argv;
{
	register char *cp;

	if (argc < 3) {
		write(2, "Usage: /etc/glob cmd pattern...\n", 32);
		return 1;
	}
	argv++;
	*av++ = *argv;
	while (--argc >= 2)
		expand(*++argv);
	if (ncoll == 0) {
		write(2, "No match\n", 9);
		return 1;
	}
	execute(ava[1], &ava[1]);
	cp = cat("/usr/bin/", ava[1]);
	execute(cp+4, &ava[1]);
	execute(cp, &ava[1]);
	write(2, "Command not found.\n", 19);
	return 0;
}

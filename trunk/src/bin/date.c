/*
 * Display or set date and time.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int	dmsize[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
long	timbuf;
char	*cbp;

/*
 * Accurate only for the past couple of centuries;
 * that will probably do.
 */
int dysize(y)
	int y;
{
	if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))
		return 366;
	return 365;
}

int
gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if (*cp == 0)
		return -1;
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return -1;
	if (*cp == 0)
		return -1;
	if ((d = *cp++ - '0') < 0 || d > 9)
		return -1;
	cbp = cp;
	return c + d;
}

void
gdadd(n)
{
	timbuf += n;
}

void
gmdadd(m, n)
{
	timbuf *= m;
	timbuf += n;
}

int
gtime()
{
	register int i;
	register int y, t;
	int d, h, m;
	long nt;

	t = gpair();
	if (t<1 || t>12)
		return 1;
	d = gpair();
	if (d<1 || d>31)
		return 1;
	h = gpair();
	if (h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if (m<0 || m>59)
		return 1;
	y = gpair();
	if (y<0) {
		time(&nt);
		y = localtime(&nt)->tm_year;
	}
	if (*cbp == 'p')
		h += 12;
	if (h<0 || h>23)
		return 1;
	timbuf = 0;
	y += 1900;
	for (i=1970; i<y; i++)
		gdadd(dysize(i));
	if (dysize(i) == 366)
		dmsize[1] = 29;
	else
		dmsize[1] = 28;
	while (--t)
		gdadd(dmsize[t-1]);
	gdadd(d-1);
	gmdadd(24, h);
	gmdadd(60, m);
	gmdadd(60, 0);
	return 0;
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	register char *tzn;
	char *cbuf;

	if (argc > 1) {
		cbp = argv[1];
		if (gtime()) {
			write(1, "bad conversion\n", 15);
			return 1;
		}
		/* convert to Greenwich time, on assumption of Standard time. */
		timbuf += timezone;
		/* Now fix up to local daylight time. */
		if (localtime(&timbuf)->tm_isdst)
			timbuf -= 1 * 60 * 60;
		if (stime(&timbuf) < 0){
			write(1, "no permission\n", 14);
			return 1;
		}
	}
	time(&timbuf);
	cbuf = ctime(&timbuf);
	write(1, cbuf, 20);
	tzn = tzname[localtime(&timbuf)->tm_isdst];
	if (tzn)
		write(1, tzn, 3);
	write(1, cbuf + 19, 6);
	return 0;
}

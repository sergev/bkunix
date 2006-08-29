int	timbuf[2];
char	*cbp;

char *tzname[2];
int	dmsize[];
char	cbuf[];
char	*cbp;

struct {
	char	name[8];
	char	ln[2];
	int	wtime[2];
	int	pid;
} wtmp[2];

main(argc, argv)
int argc, **argv;
{
	register char *tzn;
	register int i;
	extern int timezone, *localtime();
	int wf;

	if(argc > 1) {
		cbp = argv[1];
		if(gtime()) {
			write(1, "bad conversion\n", 15);
			exit();
		}
	/* convert to Greenwich time, on assumption of Standard time. */
		dpadd(timbuf, timezone);
	/* Now fix up to local daylight time. */
		if (localtime(timbuf)[8])
			dpadd(timbuf, -1*60*60);
		time(wtmp[0].wtime);
		for (i=0; wtmp[0].name[i]="OLDTIME"[i]; i++);
		wtmp[0].ln[0] = 'O';
		wtmp[0].ln[1] = 'T';
		if(stime(timbuf) < 0){
			write(1, "no permission\n", 14);
			exit(1);
		}
		if ((wf = open("/etc/wtmp", 1)) >= 0) {
			time(wtmp[1].wtime);
			for (i=0; wtmp[1].name[i]="NEWTIME"[i]; i++);
			wtmp[1].ln[0] = 'N';
			wtmp[1].ln[1] = 'T';
			wtmp[1].pid = getuid()&0377;
			seek(wf, 0, 2);
			write(wf, wtmp, 32);
		}
	}
	time(timbuf);
	cbp = cbuf;
	ctime(timbuf);
	write(1, cbuf, 20);
	tzn = tzname[localtime(timbuf)[8]];
	if (tzn)
		write(1, tzn, 3);
	write(1, cbuf+19, 6);
}

gtime()
{
	register int i;
	register int y, t;
	int d, h, m;
	extern int *localtime();
	int nt[2];

	t = gpair();
	if(t<1 || t>12)
		goto bad;
	d = gpair();
	if(d<1 || d>31)
		goto bad;
	h = gpair();
	if(h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if(m<0 || m>59)
		goto bad;
	y = gpair();
	if (y<0) {
		time(nt);
		y = localtime(nt)[5];
	}
	if (*cbp == 'p')
		h =+ 12;
	if (h<0 || h>23)
		goto bad;
	timbuf[0] = 0;
	timbuf[1] = 0;
	y =+ 1900;
	for(i=1970; i<y; i++)
		gdadd(dysize(i));
	if( dysize(i) == 366)
		dmsize[1]= 29;
	while(--t)
		gdadd(dmsize[t-1]);
	dmsize[1]= 28;
	gdadd(d-1);
	gmdadd(24, h);
	gmdadd(60, m);
	gmdadd(60, 0);
	return(0);

bad:
	return(1);
}

gdadd(n)
{
	register char *t;

	t = timbuf[1]+n;
	if(t < timbuf[1])
		timbuf[0]++;
	timbuf[1] = t;
}

gmdadd(m, n)
{
	register int t1;

	timbuf[0] =* m;
	t1 = timbuf[1];
	while(--m)
		gdadd(t1);
	gdadd(n);
}

gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

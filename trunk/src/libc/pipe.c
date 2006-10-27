/*
 * A function that behaves almost like a pipe(2) syscall.
 * Uses a file name unique to the process, so it is not
 * re-entrant (better not call from signal handlers).
 */
char pf[] = "._pfXX";
int pipe(pv)
int pv[2];
{
	register int pid = getpid();
	pf[4] = 'a' + (pid & 15);
	pid >>= 4;
	pf[5] = 'a' + (pid & 15);
	if (-1 == (close(creat(pf,0666))) ||
	    -1 == (pv[0] = open(pf,0)) ||
	    -1 == (pv[1] = open(pf,1)))
		return -1;
	unlink(pf);
	return 0;
}

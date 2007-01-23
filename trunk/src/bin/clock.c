#include <time.h>
#include <signal.h>

int go = 1;

void intr()
{
	go = 0;
}

int main()
{
	long t;
	register char *s;

	signal(SIGINT, intr);
	signal(SIGQUIT, intr);

	/* Clear screen, go to the middle, set wide font */
	write(1, "\f\n\n\n\n\n\n\n\n\n\n\n", 12);
#ifdef BK0011
	write(1, "\33\63", 2);
#else
	write(1, "\233", 1);
#endif
	while (go) {
		time(&t);
		s = ctime(&t);

		/* imitate \r */
#ifdef BK0011
		write(1, "\33\101\33\122    ", 8);
#else
		write(1, "\32    ", 5);
#endif
		write(1, s, 25);
		sleep(1);
	}
	/* Set narrow font */
#ifdef BK0011
	write(1, "\33\64\n", 3);
#else
	write(1, "\233\n", 2);
#endif
	return 0;
}

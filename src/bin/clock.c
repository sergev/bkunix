#include <time.h>
#include <signal.h>

int go = 1;

void intr() {
	go = 0;
}

int main()
{
	long t;

	signal(SIGINT, intr);
	signal(SIGQUIT, intr);

	/* Clear screen, go to the middle, set wide font */
	write(1, "\f\n\n\n\n\n\n\n\n\n\n\n\233", 13);
	while (go) {
		time(&t);
		/* imitate \r */
		printf("\032    %s", ctime(&t));
		sleep(1);
	}
	/* Set narrow font */
	write(1, "\233\n", 2);
	return 0;
}

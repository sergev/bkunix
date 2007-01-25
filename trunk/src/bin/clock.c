#include <time.h>
#include <signal.h>

#define BK0011_CLEAR	"\33\105"
#define BK0011_WIDE	"\33\63"
#define BK0011_NARROW	"\33\64"
#define BK0011_UP	"\33\101"

#define BK0010_CLEAR	"\f"
#define BK0010_WIDE	"\233"
#define BK0010_NARROW	"\233"
#define BK0010_UP	"\32"

int go = 1;

void intr()
{
	go = 0;
}

/*
 * Detect machine architecture.
 * Return 0 for BK-0010 and 1 for BK-0011/0011M.
 */
int detect_bk0011 ()
{
	/* Test word at address 100000.
	 * On BK-0010 it contains 0167 - "jmp" instruction.
	 * In case of BK-0011 the kernel is placed here which has
	 * different value at this address. */
	return *((int*) 0100000) != 0167;
}

int main()
{
	long t;
	register char *s;
	register int bk0011;

	bk0011 = detect_bk0011();
	signal(SIGINT, intr);
	signal(SIGQUIT, intr);

	/* Clear screen, go to the middle, set wide font */
	if (bk0011)
		write(1, BK0011_CLEAR, sizeof(BK0011_CLEAR) - 1);
	else
		write(1, BK0010_CLEAR, sizeof(BK0010_CLEAR) - 1);

	write(1, "\n\n\n\n\n\n\n\n\n\n\n\n", 12);

	if (bk0011)
		write(1, BK0011_WIDE, sizeof(BK0011_WIDE) - 1);
	else
		write(1, BK0010_WIDE, sizeof(BK0010_WIDE) - 1);

	while (go) {
		time(&t);
		s = ctime(&t);

		/* Line up */
		if (bk0011)
			write(1, BK0011_UP, sizeof(BK0011_UP) - 1);
		else
			write(1, BK0010_UP, sizeof(BK0010_UP) - 1);

		write(1, "    ", 4);
		write(1, s, 25);
		sleep(1);
	}
	/* Set narrow font */
	if (bk0011)
		write(1, BK0011_NARROW, sizeof(BK0011_NARROW) - 1);
	else
		write(1, BK0010_NARROW, sizeof(BK0010_NARROW) - 1);

	write(1, "\n", 1);
	return 0;
}

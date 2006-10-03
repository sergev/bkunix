/*
 * Returns 1 iff file is a tty
 */
#include <unistd.h>
#include <sgtty.h>

int
isatty(f)
{
	struct sgttyb ttyb;

	if (gtty(f, &ttyb) < 0)
		return 0;
	return 1;
}

/*
 * Returns 1 iff file is a tty
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
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

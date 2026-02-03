/*
 * Returns 1 iff file is a tty
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
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

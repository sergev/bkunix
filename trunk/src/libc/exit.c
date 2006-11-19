/*
 * Close stdio files and terminate the program.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>

void (*_exitfunc)();

void
exit(code)
	int code;
{
	if (_exitfunc)
		(*_exitfunc)();
	_exit(code);
}

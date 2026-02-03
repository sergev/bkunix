/*
 * Close stdio files and terminate the program.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
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

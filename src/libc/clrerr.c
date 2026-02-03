/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

void
clearerr(iop)
	register struct _iobuf *iop;
{
	iop->_flag &= ~(_IOERR|_IOEOF);
}

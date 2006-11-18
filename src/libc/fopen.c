/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

extern FILE *_lastbuf;

static int
create(file, rw)
	register char *file;
	int rw;
{
	register int f;

	f = creat(file, 0666);
	if (rw && f >= 0) {
		close(f);
		f = open(file, 2);
	}
	return f;
}

FILE *
_endopen(file, mode, iop)
	char *file, *mode;
	register FILE *iop;
{
	register int rw, f;

	if (iop == NULL)
		return NULL;

	rw = mode[1] == '+';

	switch (*mode) {

	case 'w':
		f = create(file, rw);
		break;

	case 'a':
		f = open(file, rw ? 2 : 1);
		if (f < 0)
			f = create(file, rw);
		lseek(f, 0L, 2);
		break;

	case 'r':
		f = open(file, rw? 2: 0);
		break;

	default:
		return NULL;
	}

	if (f < 0)
		return NULL;

	iop->_cnt = 0;
	iop->_file = f;

	if (rw)
		iop->_flag |= _IORW;
	else if (*mode == 'r')
		iop->_flag |= _IOREAD;
	else
		iop->_flag |= _IOWRT;

	return iop;
}

FILE *
_findiop()
{
	register FILE *iop;

	for (iop = _iob; iop->_flag & (_IOREAD|_IOWRT|_IORW); iop++)
		if (iop >= _lastbuf)
			return NULL;

	return iop;
}

FILE *
fopen(file, mode)
	char *file, *mode;
{
	return _endopen(file, mode, _findiop());
}

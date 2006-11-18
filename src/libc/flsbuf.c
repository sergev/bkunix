/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>

char _sibuf[BUFSIZ];
char _sobuf[BUFSIZ];

FILE _iob[_NFILE] = {
	{ _sibuf,	0,	_sibuf,	_IOREAD,	0},
	{ NULL,		0,	NULL,	_IOWRT,		1},
	{ NULL,		0,	NULL,	_IOWRT+_IONBF,	2},
};

/*
 * Ptr to end of buffers
 */
FILE *_lastbuf = &_iob[_NFILE];

int
_flsbuf(c, iop)
	int c;
	register FILE *iop;
{
	register char *base;
	register n, rn;
	char c1;

	if (iop->_flag & _IORW) {
		iop->_flag |= _IOWRT;
		iop->_flag &= ~_IOEOF;
	}

tryagain:
	if (iop->_flag & _IONBF) {
		c1 = c;
		rn = 1;
		n = write(fileno(iop), &c1, rn);
		iop->_cnt = 0;
	} else {
		base = iop->_base;
		if (base == NULL) {
			if (iop == stdout) {
				if (isatty(fileno(stdout))) {
					iop->_flag |= _IONBF;
					goto tryagain;
				}
				iop->_base = _sobuf;
				iop->_ptr = _sobuf;
				goto tryagain;
			}
			base = iop->_base = malloc(BUFSIZ);
			if (! base) {
				iop->_flag |= _IONBF;
				goto tryagain;
			}
			iop->_flag |= _IOMYBUF;
			rn = n = 0;
		} else if((rn = n = iop->_ptr - base) > 0) {
			iop->_ptr = base;
			n = write(fileno(iop), base, n);
		}
		iop->_cnt = BUFSIZ - 1;
		*base++ = c;
		iop->_ptr = base;
	}
	if (rn != n) {
		iop->_flag |= _IOERR;
		return(EOF);
	}
	return(c);
}

int
fflush(iop)
	register FILE *iop;
{
	register char *base;
	register n;

	if ((iop->_flag & (_IONBF|_IOWRT)) == _IOWRT
	 && (base = iop->_base) != NULL && (n = iop->_ptr - base) > 0) {
		iop->_ptr = base;
		iop->_cnt = BUFSIZ;
		if (write(fileno(iop), base, n) != n) {
			iop->_flag |= _IOERR;
			return(EOF);
		}
	}
	return(0);
}

/*
 * Flush buffers on exit
 */
_cleanup()
{
	register FILE *iop;

	for (iop = _iob; iop < _lastbuf; iop++)
		fclose(iop);
}

int
fclose(iop)
	register FILE *iop;
{
	register r;

	r = EOF;
	if (iop->_flag & (_IOREAD|_IOWRT|_IORW)
	    && (iop->_flag & _IOSTRG) == 0) {
		r = fflush(iop);
		if (close(fileno(iop)) < 0)
			r = EOF;
		if (iop->_flag & _IOMYBUF)
			free(iop->_base);
		if (iop->_flag & (_IOMYBUF|_IONBF))
			iop->_base = NULL;
	}
	iop->_flag &=
		~(_IOREAD|_IOWRT|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG|_IORW);
	iop->_cnt = 0;
	return(r);
}

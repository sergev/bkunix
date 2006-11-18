/*
 * Standard input/output library functions.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _STDIO_H_
#define _STDIO_H_ 1

#include <ansidecl.h>

#define	BUFSIZ		512
#define	_NFILE		20

struct _iobuf {
	char	*_ptr;
	int	_cnt;
	char	*_base;
	char	_flag;
	char	_file;
};

#define	_IOREAD		01
#define	_IOWRT		02
#define	_IONBF		04
#define	_IOMYBUF	010
#define	_IOEOF		020
#define	_IOERR		040
#define	_IOSTRG		0100
#define	_IORW		0200

#define	NULL		0
#define	FILE		struct _iobuf
#define	EOF		(-1)

#define	stdin		(&_iob[0])
#define	stdout		(&_iob[1])
#define	stderr		(&_iob[2])
#define	getc(p)		(--(p)->_cnt >= 0 ? *(p)->_ptr++ & 0377 : _filbuf(p))
#define	getchar()	getc(stdin)
#define putc(x,p)	(--(p)->_cnt >= 0 ? \
				(unsigned char) (*(p)->_ptr++ = (x)) : \
				_flsbuf((unsigned)(x), p))
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag & _IOEOF) != 0)
#define	ferror(p)	(((p)->_flag & _IOERR) != 0)
#define	fileno(p)	p->_file

extern FILE _iob[_NFILE];

FILE *fopen PARAMS((char*, char*));
FILE *freopen PARAMS((char*, char*, FILE*));
FILE *fdopen PARAMS((int, char*));
long ftell PARAMS((FILE*));
char *fgets PARAMS((char*, int, FILE*));

int printf PARAMS(());
int fprintf PARAMS(());
int sprintf PARAMS(());
int vprintf PARAMS((char*, char*));
int vfprintf PARAMS((FILE*, char*, char*));

#endif /* _STDIO_H_ */

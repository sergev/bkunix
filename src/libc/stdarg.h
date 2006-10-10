/*
 * Variable argument lists.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _STDARG_H_
#define _STDARG_H_ 1

#define va_list		char*
#define va_start(v,a)	{ v = (char*) &(a) + (sizeof(a) > sizeof(int) ? \
				sizeof(a) : sizeof(int)); }
#define va_end(v)
#define va_arg(v,t)	((t*) (v += sizeof(t)))[-1]

#endif /* _STDARG_H_ */

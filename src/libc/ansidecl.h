/*
 * ANSI and traditional C compatability macros.
 * ANSI C is assumed if __STDC__ is #defined.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef	_ANSIDECL_H_
#define _ANSIDECL_H_ 1

#ifndef PARAMS
#   ifdef __STDC__
#      define ANSI_PROTOTYPES	1
#      define PARAMS(args)	args
#   else
#      undef  ANSI_PROTOTYPES
#      define PARAMS(args) 	()
#   endif
#endif

#undef const
#undef volatile
#undef signed
#undef inline
#define const
#define volatile
#define signed
#define inline

#endif	/* _ANSIDECL_H_ */

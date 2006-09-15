#ifndef _MACDEFS_
#define	_MACDEFS_

#define makecc(val,i)  lastcon = i ? (val<<8)|lastcon : val  

#define ARGINIT		32 
#define AUTOINIT	0 

/*
 * Storage space requirements
 */
#define SZCHAR		8
#define SZINT		16
#define SZFLOAT		32
#define SZDOUBLE	64
#define SZLONG		32
#define SZSHORT		16
#define SZPOINT		16

/*
 * Alignment constraints
 */
#define ALCHAR		8
#define ALINT		16
#define ALFLOAT		16
#define ALDOUBLE	16
#define ALLONG		16
#define ALSHORT		16
#define ALPOINT		16
#define ALSTRUCT	16
#define ALSTACK		16

typedef	long	CONSZ;		/* size in which constants are converted */
typedef	long	OFFSZ;		/* size in which offsets are kept */

#define CONFMT	"%ld"		/* format for printing constants */
#define LABFMT	"L%d"		/* format for printing labels */

#define CCTRANS(x) x		/* character set macro */

/*
 * Register cookies for stack pointer and argument pointer
 */

# define STKREG 5
# define ARGREG 5
/*
 * Maximum and minimum register variables
 */
# define MAXRVAR 4
# define MINRVAR 2

#define BACKAUTO		/* stack grows negatively for automatics */
#define BACKTEMP		/* stack grows negatively for temporaries */
#ifdef	vax
#define FIELDOPS		/* show field hardware support on VAX */
#endif
#define RTOLBYTES		/* bytes are numbered from right to left */

#define ENUMSIZE(high,low) INT	/* enums are always stored in full int */

#define ADDROREG
#define FIXDEF(p) outstab(p)
#define FIXARG(p) fixarg(p)
#ifndef ncopy
#define	ncopy(q, p)	((q)->in = (p)->in)
#endif
#endif

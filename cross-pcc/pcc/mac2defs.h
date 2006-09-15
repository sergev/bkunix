/*
 * PDP11 Registers
 */

/*
 * Scratch registers
 */
#define R0	0
#define R1	1

/*
 * Register variables
 */
#define R2	2
#define R3	3
#define R4	4

/*
 * Special purpose registers
 */
#define R5	5	/* frame pointer */
#define SP	6	/* stack pointer */
#define PC	7	/* program counter */

/* floating registers */

#define FR0	8
#define FR1	9
#define FR2	10
#define FR3	11
#define FR4	12
#define FR5	13

#define REGSZ	14
#define TMPREG	R5

extern	int fregs;
extern	int maxargs;

#define BYTEOFF(x) ((x)&01)
#define wdal(k)		(BYTEOFF(k)==0)		/* word align */
#define BITOOR(x)	((x)>>3)		/* bit offset to oreg offset */

/*
 * Some macros used in store():
 *	just evaluate the arguments, and be done with it...
 */
#define STOARG(p)
#define STOFARG(p)
#define STOSTARG(p)
#define genfcall(a,b)	gencall(a,b)

/*
 * Some short routines that get called an awful lot are actually macros.
 */
#define	shltype(o, p) \
	((o) == REG || (o) == NAME || (o) == ICON || \
	 (o) == OREG || ((o) == UNARY MUL && shumul((p)->in.left)))
#define	ncopy(q, p)	((q)->in = (p)->in)

#define MYREADER(p) myreader(p)
int	optim2();

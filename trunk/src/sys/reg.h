/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef REG_H
#define REG_H 1

/*
 * Location of the users' stored
 * registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	R0	(0)
#define	R1	(-2)
#define	R2	(-9)
#define	R3	(-8)
#define	R4	(-7)
#define	R5	(-6)
#define	R6	(-3)
#define	R7	(1)
#define	RPS	(2)

#define	TBIT	020		/* PS trace bit */

#endif /* REG_H */

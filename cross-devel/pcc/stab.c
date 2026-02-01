/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "pass1.h"
/*
 * No op the stab stuff for now
*/

int
psline(void)
{
	return(0);
}

/* ARGSUSED */
int
pstab(struct symtab *p, int n)
{
	(void)p;
	(void)n;
	return(0);
}

/* ARGSUSED */
int
outstab(struct symtab *p)
{
	(void)p;
	return(0);
}

/* ARGSUSED */
int
fixarg(struct symtab *p)
{
	(void)p;
	return(0);
}

/* ARGSUSED */
int
outstruct(struct symtab *p)
{
	(void)p;
	return(0);
}

/* ARGSUSED */
int
pfstab(char *cp)
{
	(void)cp;
	return(0);
}

/* ARGSUSED */
int
plcstab(int level)
{
	(void)level;
	return(0);
}

/* ARGSUSED */
int
prcstab(int level)
{
	(void)level;
	return(0);
}

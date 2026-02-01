/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "pass1.h"
/*
 * No op the stab stuff for now
*/

void
psline(void)
{
	return;
}

/* ARGSUSED */
void
pstab(const void *p, int n)
{
	(void)p;
	(void)n;
	return;
}

/* ARGSUSED */
void
outstab(struct symtab *p)
{
	(void)p;
	return;
}

/* ARGSUSED */
void
fixarg(struct symtab *p)
{
	(void)p;
	return;
}

/* ARGSUSED */
void
outstruct(int szindex, int oparam)
{
	(void)szindex;
	(void)oparam;
	return;
}

/* ARGSUSED */
void
pfstab(const char *cp)
{
	(void)cp;
	return;
}

/* ARGSUSED */
void
plcstab(int level)
{
	(void)level;
	return;
}

/* ARGSUSED */
void
prcstab(int level)
{
	(void)level;
	return;
}

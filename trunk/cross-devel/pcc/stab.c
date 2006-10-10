/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "pass1.h"
/*
 * No op the stab stuff for now
*/

psline()
	{
	return(0);
	}

/* ARGSUSED */
pstab(p,n)
	struct symtab *p;
	int	n;
	{
	return(0);
	}

/* ARGSUSED */
outstab(p)
	struct symtab *p;
	{
	return(0);
	}

/* ARGSUSED */
fixarg(p)
	struct symtab *p;
	{
	return(0);
	}

/* ARGSUSED */
outstruct(p)
	struct symtab *p;
	{
	return(0);
	}

/* ARGSUSED */
pfstab(cp)
	char	*cp;
	{
	return(0);
	}

/* ARGSUSED */
plcstab(level)
	int	level;
	{
	return(0);
	}

/* ARGSUSED */
prcstab(level)
	int	level;
	{
	return(0);
	}

/*
 * AS - PDP/11 Assembler, Part II
 *
 * Miscellaneous support routines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as2.h"

/*
	Routine to output a word to output, with relocation
*/
void outw(type, val)
	int type, val;
{
	unsigned t;

	if(DEBUG)
		printf("outw type %o val %o ",type,val);
	if(dotrel == TYPEBSS) {
		aerror('x');
		return;
	}
	if(dot & 1) {
		aerror('o');
		outb(TYPEUNDEF,val);
		return;
	}
	dot += 2;
	if(passno == 0)
		return;
	t = ((type & ENDTABFLAG) >> 15) & 1;
	type &= ~ENDTABFLAG;
	if(type == TYPEEXT) {
		outmod = 0666;
		type = (((struct value *)xsymbol - usymtab) << 3) | 4;
	}
	else {
		if((type &= ~TYPEEXT) >= TYPEOPFD) {
			if(type == TYPEOPEST || type == TYPEOPESD)
				aerror('r');
			type = TYPEABS;
		}
		if(type >= TYPETXT && type <= TYPEBSS) {
			if(t == 0)
				val += dotdot;
		}
		else {
			if(t != 0)
				val -= dotdot;
		}
		if(--type < 0)
			type = TYPEUNDEF;
	}

	type = (type << 1) | t;
	aputw(&txtp,val);
	*tseekp += 2;
	aputw(&relp,type);
	*rseekp += 2;
	return;
}


/*
	Routine to output a byte value
*/
void outb(type, val)
	int type, val;
{
	if(dotrel == TYPEBSS) {
		aerror('x');
		return;
	}
	if(type > TYPEABS)
		aerror('r');
	if(passno != 0) {
		if(!(dot & 1)) {
			aputw(&txtp,val);
			aputw(&relp,0);
			*rseekp += 2;
			*tseekp += 2;
		}
		else {
			*((char *)txtp.slot-1) = val;
		}
	}
	++dot;
}


/*
	Display file, line and error code for errors
*/
void aerror(c)
	int c;
{
	char *msg = 0;

	switch (c) {
	case 'u': msg = "Unknown symbol"; break;
	case 'x': msg = "Data in .bss section"; break;
	case 'o': msg = "Odd address"; break;
	case 'r': msg = "Relocatable value not allowed"; break;
	case '.': msg = "Dot-relative expression required"; break;
	case 'b': msg = "Branch offset too big"; break;
	case 'a': msg = "Incorrect operand value"; break;
	case ')': msg = "Requred ')'"; break;
	case ']': msg = "Requred ']'"; break;
	case 'e': msg = "Bad expression"; break;
	}
	printf("%s:%d: ", argb, line);
	if (msg)
		printf("%s\n", msg);
	else
		printf("Error '%c'\n", c);

	outmod = 0666;		/* not executable */
}

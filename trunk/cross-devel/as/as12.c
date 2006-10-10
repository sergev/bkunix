/*
 * as - PDP/11 assember, Part 1
 *
 * Some support routines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as1.h"

/*
	Print an error message
*/
void aerror(c)
	int c;
{
	char *msg = 0;

	switch (c) {
	case 'x': msg = "Syntax error"; break;
	case '.': msg = "Dot '.' expected"; break;
	case 'm': msg = "Invalid label"; break;
	case 'f': msg = "Invalid temporary label"; break;
	case 'i': msg = "Unterminated .endif"; break;
	case 'g': msg = "Unexpected character"; break;
	case '<': msg = "Unterminated <> string"; break;
	case 'a': msg = "Invalid register name"; break;
	case 'U': msg = "Undefined identifier in .if"; break;
	case '~': msg = "Internal error"; break;
	case '*': msg = "Error at '*'"; break;
	case ')': msg = "Expected ')'"; break;
	case 'e': msg = "Invalid expression"; break;
	case ']': msg = "Expected ']'"; break;
	}
	printf("%s:%d: ", *curarg, line);
	if (msg)
		printf("%s\n", msg);
	else
		printf("Error '%c'\n", c);
	++errflg;
}


/*
	Put current token to token output, except when
	inside a .if and it isn't a newline (to keep line count)
*/
void aputw()
{
	char buf[2];

	if(!ifflg || tok.i == '\n') {
		buf[0] = tok.i;
		buf[1] = tok.i >> 8;
		if(write(pof, buf, 2) != 2)
			fprintf(stderr,"aputw: write error\n");
	}
}

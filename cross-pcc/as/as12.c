/*
	as - PDP/11 assember, Part 1

	Some support routines
*/

#include <stdio.h>
#include "as.h"
#include "as1.h"

/*
	Print an error message
*/
void aerror(c)
char c;
{
	printf("%s %c %d\n",*curarg,c,line);
	++errflg;
}


/*
	Put current token to token output, except when 
	inside a .if and it isn't a newline (to keep line count)
*/
void aputw()
{
	if(!ifflg || tok.i == '\n') {
		if(write(pof,&tok.i,2) != 2)
			fprintf(stderr,"aputw: write error\n");
	}
}

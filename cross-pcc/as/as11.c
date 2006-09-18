/*
	as - PDP/11 Assember, Part 1 - main program
*/

#include <stdio.h>
#include "as.h"
#include "as1.h"

int
main(argc,argv)
	int argc;
	char *argv[];
{
	int fsym;

	globflag = TRUE;
	if(*argv[1] == '-') {
		++argv;
		--argc;
	}
	else
		globflag = FALSE;
	nargs = argc;
	curarg = argv;
	pof = f_create(ATMP1);
	fbfil = f_create(ATMP2);
	fin = 0;
	memset(hshtab, 0, sizeof hshtab);
	setup();
	ch = 0;
	ifflg = 0;
	fileflg = 0;
	errflg = 0;
	savop = 0;
	assem();
	close(pof);
	close(fbfil);
	if(errflg)
		aexit();
	fsym = f_create(ATMP3);
	write(fsym,usymtab,(char *)symend - (char *)usymtab);
	close(fsym);
	exit(0);
}


/*
	Routine to "handle" an error on a file
*/
void filerr(name,msg)
	char *name, *msg;
{
	fprintf(stderr,"%s %s\n",name,msg);
	return;
}


/*
	Routine to exit program (without doing pass 2)
*/
void
aexit()
{
/*
	unlink(ATMP1);
	unlink(ATMP2);
	unlink(ATMP3);
*/
	exit(1);
}


/*
	Routine to create one of the temporary files
*/
int f_create(name)
	char *name;
{
	int fd;

	if((fd = creat(name, 0600)) < 0) {
		filerr(name,"f_create: can't create file.");
		exit(2);
	}
	return(fd);
}


/*
	Routine to build permanent symbol table
*/
void setup()
{
	int n;
	struct symtab *p,*e;
	FILE * fd;

	if((fd = fopen(OPTABL,"r")) == 0) {
		fprintf(stderr,"setup: can't open %s.\n",OPTABL);
		exit(2);
	}
	p = symtab;
	while(p - symtab < SYMBOLS &&
		  (n = fscanf(fd,"%s %o %o",p->name,&p->v.type.i,&p->v.val.i)) == 3) {
		  ++p;
	}
	if(p - symtab > SYMBOLS) {
		fprintf(stderr,"setup: permanent symbol table overflow.\n");
		exit(2);
	}
	if(n != -1) {
		fprintf(stderr,"setup: scanned only %d elements after %d symbols.\n",
			n,p - symtab);
		exit(2);
	}
	for(e=p, p=symtab; p < e; ++p) {
		memset(p->name+strlen(p->name), 0, 8-strlen(p->name));
		hash_enter(p);
	}
	fclose(fd);
}

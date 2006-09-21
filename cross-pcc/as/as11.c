/*
	as - PDP/11 Assember, Part 1 - main program
*/

#include <stdio.h>
#include "as.h"
#include "as1.h"

char atmp1[] = "/tmp/atm1XXXXXX";
char atmp2[] = "/tmp/atm2XXXXXX";
char atmp3[] = "/tmp/atm3XXXXXX";

char *outfile;

void
usage()
{
	fprintf(stderr, "Usage: asm [-u] [-o outfile] [infile...]\n");
	exit(1);
}

int
main(argc,argv)
	int argc;
	char *argv[];
{
	int fsym, n, debug = 0;
	char *av[8];

	while(argv[1] && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'u':
			/* Option -u: treat undefined name as external */
			globflag = TRUE;
			break;
		case 'D':
			/* Option -D: debug mode (do not run pass 2) */
			debug = 1;
			break;
		case 'o':
			outfile = argv[2];
			if (! outfile)
				usage();
			++argv;
			--argc;
			break;
		default:
			usage();
		}
		++argv;
		--argc;
	}
	nargs = argc;
	curarg = argv;
	pof = f_create(atmp1);
	fbfil = f_create(atmp2);
	fin = argc > 1 ? 0 : stdin;
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
	fsym = f_create(atmp3);
	write_syms(fsym);
	close(fsym);

	/* Run pass 2. */
	if (debug)
		exit(0);
	n = 0;
	av[n++] = PASS2;
	if (globflag)
		av[n++] = "-u";
	if (outfile) {
		av[n++] = "-o";
		av[n++] = outfile;
	}
	av[n++] = atmp1;
	av[n++] = atmp2;
	av[n++] = atmp3;
	av[n] = 0;
	execv(PASS2, av);
	fprintf(stderr, "Could not exec %s\n", PASS2);
	exit(1);
}

void write_syms (fd)
	int fd;
{
	struct symtab *s;
	char buf[4];

	for (s=usymtab; s<symend; ++s) {
		write(fd, s->name, sizeof(s->name));
		buf[0] = s->v.type.u;
		buf[1] = s->v.type.u >> 8;
		buf[2] = s->v.val.u;
		buf[3] = s->v.val.u >> 8;
		write(fd, buf, 4);
	}
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
	unlink(atmp1);
	unlink(atmp2);
	unlink(atmp3);
	exit(1);
}


/*
	Routine to create one of the temporary files
*/
int f_create(name)
	char *name;
{
	int fd;

	if((fd = mkstemp(name)) < 0) {
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

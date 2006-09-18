/*	
	AS - PDP/11 assember, Part II

	Main program and associated routines
*/

#include <stdio.h>
#include "as.h"
#include "as2.h"

/*
	Main program.
*/

int
main(argc,argv)
int argc;
char *argv[];
{
	struct value *sp,*p;			/* Pointer into symbol table*/
	unsigned t;
	struct fb_tab *fp;
	int *pi,i;

	if(argc > 1)
		defund = TYPEEXT;
	txtfil = ofile(ATMP1);
	fbfil  = ofile(ATMP2);
	symf   = ofile(ATMP3);
	fin = symf;
	if((fout = creat("a.out")) <= 0)
		filerr("a.out");

	/*
		Read in the symbol table, dropping the name part
	*/
	sp = usymtab;
	while(agetw()) {
		hdr.symsiz += 12;
		agetw(); agetw(); agetw(); agetw();
		t = tok.u & 037;
		if(t < TYPETXT || t > TYPEDATA) {
			sp->type.i = TYPEUNDEF;
			sp->val.i = 0;
			agetw();
		}
		else {
			sp->type.i = tok.i - TYPETXT + TYPEOPEST;
			agetw();
			sp->val.u = tok.u;
		}
		++sp;
	}

	/*
		Read in forward branch table
	*/
	fp = fbbufp = fbtab;	/* was on end of symbol table... */
	fin = fbfil;
	while(agetw()) {
		fp->rel = (tok.u & 0xff) - TYPETXT + TYPEOPEST;
		fp->lblix = (tok.u >> 8) & 0xff;
		agetw();
		fp->val = tok.i;
		if(DEBUG)
			printf("fbsetup %d type %o value %x\n",fp->lblix,
				fp->rel,fp->val);
		++fp;
	}
	endtable = fp;
	((struct value *)fp)->type.u = ENDTABFLAG;

	/*
		Do pass 2	(pass 0 of second phase...)
	*/
	setup();
	assem();
	if(outmod != 0777)
		aexit();

	/*
		Now set up for pass 3, including header for a.out
	*/
	dot = 0;
	dotrel = TYPETXT;
	dotdot = 0;
	brtabp = 0;
	close(fin);
	fin = ofile(ATMP1);
	++passno;
	setup();
	bsssiz = (bsssiz + 1) & ~1;
	txtsiz = (txtsiz + 1) & ~1;
	datsiz = (datsiz + 1) & ~1;
	datbase = txtsiz;
	savdot[1] = datbase;		/* .data goes after text */
	bssbase = txtsiz + datsiz;
	savdot[2] = bssbase;		/* .bss after text, data */

	symseek  = 2*txtsiz + 2*datsiz + 020;
	drelseek = 2*txtsiz + datsiz + 020;
	trelseek = txtsiz + datsiz + 020;
	datseek  = txtsiz + 020;
	txtseek = 020;

	for(p = usymtab; p < sp; ++p)
		doreloc(p);

	/*
		We have to relocate fb-table separately, since we moved
		it off from the end of the symbol table.
	*/
	for(p = fbtab; p < endtable; ++p)
		doreloc(p);

	oset(&txtp,0);
	oset(&relp,trelseek);
	for(i=8, pi = (int *)&hdr; i > 0; --i, ++pi) {
		aputw(&txtp,*pi);
	}

	assem();

	/*
		Flush buffers, append symbol table, close output
	*/
	flush(&txtp);
	flush(&relp);
	fin = symf;
	lseek(fin,0L,0);
	oset(&txtp,symseek);
	sp = usymtab;
	while(agetw()) {
		aputw(&txtp,tok.u);
		agetw();
		aputw(&txtp,tok.u);
		agetw();
		aputw(&txtp,tok.u);
		agetw();
		aputw(&txtp,tok.u);
		aputw(&txtp,sp->type.u);
		aputw(&txtp,sp->val.u);
		++sp;
		agetw();
		agetw();
	}
	flush(&txtp);
	exit(0);
}


/*
	Routine to delete temp files and exit
*/
void aexit()
{
	/*
	unlink(ATMP1);
	unlink(ATMP2);
	unlink(ATMP3);
	*/
	/* chmod("a.out",outmod); */
	exit(1);
}


/*
	Routine to "handle" a file error
*/
void filerr(name)
char *name;
{
	printf("filerr: File error in file %s\n",name);
	aexit();
}


/*
	Routine to add appropriate relocation factor to symbol value
*/
void doreloc(p)
struct value *p;
{
	int t;

	if((t = p->type.b) == TYPEUNDEF)
		p->type.b |= defund;
	t &= 037;
	if(t >= TYPEOPFD || t < TYPEDATA)
		return;
	p->val.i += (t == TYPEDATA ? datbase : bssbase);
}


/*
	Routine to set up for a pass
*/
void setup()
{
	int i;
	int n;
	FILE * fd;
	struct value *p;
	char dummy[12];

	/*
		If first pass of this phase, read in value part of
		permanent symbol table
	*/
	if(passno == 0) {
		if((fd = fopen(OPTABL,"r")) == NULL) {
			fprintf(stderr,"setup: can't open %s\n",OPTABL);
			aexit();
		}
		p = &symtab;
		while(p-symtab < SYMBOLS &&
			  (n = fscanf(fd,"%s %o %o",dummy,
			  	&p->type.u,&p->val.u)) == 3) {
			  	++p;
		}
		if(p-symtab >= SYMBOLS) {
			fprintf(stderr,"setup: Permanent symbol table overflow\n");
			aexit();
		}
		if(n != -1) {
			fprintf(stderr,
			   "setup: scanned only %d elements after %d symbols\n",
				n,p-symtab);
			aexit();
		}
		fclose(fd);
	}

	for(i=0; i<10; ++i)
		nxtfb[i] = curfb[i] = 0;
	fin = txtfil;
	for(i=0; i<10; ++i) {
		tok.i = i;
		fbadv();
	}
}


/*
	Routine to open an input file
*/
int ofile(name)
char *name;
{
	int fd;

	if((fd = open(name,0)) < 0)
		filerr(name);
	return(fd);
}



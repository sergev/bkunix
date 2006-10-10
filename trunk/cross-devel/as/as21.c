/*
 * AS - PDP/11 assember, Part II
 *
 * Main program and associated routines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as2.h"

char *atmp1, *atmp2, *atmp3;
char *outfile = "a.out";
int debug;

void
usage()
{
	fprintf(stderr, "Usage: asm2 [-u] [-o outfile] tmpfile1 tmpfile2 tmpfile3\n");
	exit(1);
}

/*
	Main program.
*/
int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct value *sp,*p;			/* Pointer into symbol table*/
	unsigned t;
	struct fb_tab *fp;
	int *pi,i;

	while(argv[1] && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'u':
			/* Option -u: treat undefined name as external */
			defund = TYPEEXT;
			break;
		case 'D':
			/* Option -D: debug mode (leave tmp files) */
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
	if (argc < 4)
		usage();

	txtfil = ofile(atmp1 = argv[1]);
	fbfil  = ofile(atmp2 = argv[2]);
	symf   = ofile(atmp3 = argv[3]);
	fin = symf;
	fout = creat(outfile, 0644);
	if(fout <= 0)
		filerr(outfile);

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
		fp->label = (unsigned char) (tok.u - TYPETXT + TYPEOPEST);
		fp->label |= tok.u & ~0xff;
		agetw();
		fp->val = tok.i;
		if(DEBUG)
			printf("fbsetup %d type %o value %x\n", fp->label >> 8,
				(char) fp->label, fp->val);
		++fp;
	}
	endtable = fp;
	((struct value *)fp)->type.u = ENDTABFLAG;

	/*
		Do pass 2	(pass 0 of second phase...)
	*/
	setup();
	fin = txtfil;
	assem();
	if(outmod != 0777)
		aexit(1);

	/*
		Now set up for pass 3, including header for a.out
	*/
	dot = 0;
	dotrel = TYPETXT;
	dotdot = 0;
	brtabp = 0;
	++passno;
	setup();
	lseek(fin,0L,0);
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
	for(fp = fbtab; fp < endtable; ++fp)
		doreloc((struct value*) fp);

	oset(&txtp, 0);
	oset(&relp, trelseek);
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
	oset(&txtp, symseek);
	sp = usymtab;
	while(agetw()) {
		aputw(&txtp, tok.u);
		agetw();
		aputw(&txtp, tok.u);
		agetw();
		aputw(&txtp, tok.u);
		agetw();
		aputw(&txtp, tok.u);
		aputw(&txtp, sp->type.u);
		aputw(&txtp, sp->val.u);
		++sp;
		agetw();
		agetw();
	}
	flush(&txtp);
	aexit(0);
	return 0;
}


/*
	Routine to delete temp files and exit
*/
void aexit(code)
	int code;
{
	if (! debug) {
		unlink(atmp1);
		unlink(atmp2);
		unlink(atmp3);
	}
	exit(code);
}


/*
	Routine to "handle" a file error
*/
void filerr(name)
	char *name;
{
	printf("filerr: File error in file %s\n",name);
	aexit(1);
}


/*
	Routine to add appropriate relocation factor to symbol value
*/
void doreloc(p)
	struct value *p;
{
	int t;

	if((t = p->type.i) == TYPEUNDEF)
		p->type.i |= defund;
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
			aexit(1);
		}
		p = &symtab[0];
		while(p-symtab < SYMBOLS &&
			  (n = fscanf(fd,"%s %o %o",dummy,
			  	&p->type.u,&p->val.u)) == 3) {
			  	++p;
		}
		if(p-symtab >= SYMBOLS) {
			fprintf(stderr,"setup: Permanent symbol table overflow\n");
			aexit(1);
		}
		if(n != -1) {
			fprintf(stderr,
			   "setup: scanned only %d elements after %d symbols\n",
				n,p-symtab);
			aexit(1);
		}
		fclose(fd);
	}

	for(i=0; i<10; ++i)
		nxtfb[i] = curfb[i] = 0;
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

//
// AS - PDP/11 assember, Part II
//
// Main program and associated routines
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as2.h"

//
// Print pass2 usage to stderr and exit with failure.
// Called when pass2 is invoked with bad or missing arguments.
// Inputs: None.
// Outputs: None; terminates with exit(1).
//
void p2_usage(void)
{
    fprintf(stderr, "Usage: asm2 [-u] [-o outfile] tmpfile1 tmpfile2 tmpfile3\n");
    exit(1);
}

//
// Pass2 entry: read symbol table and fb table, run two assembly passes, write a.out.
// Called from main after pass1; uses atmp1/atmp2/atmp3 and outfile.
// Inputs: globflag (TYPEEXT for undefined), outfile (or "a.out"); temp files from pass1.
// Outputs: Creates a.out (or outfile); returns 0; may call p2_aexit on error.
//
int asm_pass2(int globflag, char *outfile)
{
    struct pass2 p2;
    memset(&p2, 0, sizeof(p2));
    struct value *sp, *p;
    unsigned t;
    struct fb_tab *fp;
    int *pi, i;

    pass2_init(&p2);
    p2.defund  = globflag ? TYPEEXT : 0;
    p2.outfile = outfile ? outfile : "a.out";
    p2.atmp1   = atmp1;
    p2.atmp2   = atmp2;
    p2.atmp3   = atmp3;
    p2.txtfil  = p2_ofile(&p2, p2.atmp1);
    p2.fbfil   = p2_ofile(&p2, p2.atmp2);
    p2.symf    = p2_ofile(&p2, p2.atmp3);
    p2.fin     = p2.symf;
    p2.fout    = creat(p2.outfile, 0644);
    if (p2.fout <= 0)
        p2_filerr(&p2, p2.outfile);

    // Read in the symbol table, dropping the name part
    sp = p2.usymtab;
    while (p2_agetw(&p2)) {
        p2.hdr.symsiz += 12;
        p2_agetw(&p2);
        p2_agetw(&p2);
        p2_agetw(&p2);
        p2_agetw(&p2);
        t = p2.tok.u & 037;
        if (t < TYPETXT || t > TYPEDATA) {
            sp->type.i = TYPEUNDEF;
            sp->val.i  = 0;
            p2_agetw(&p2);
        } else {
            sp->type.i = p2.tok.i - TYPETXT + TYPEOPEST;
            p2_agetw(&p2);
            sp->val.u = p2.tok.u;
        }
        ++sp;
    }

    // Read in forward branch table
    fp = p2.fbbufp = p2.fbtab;
    p2.fin         = p2.fbfil;
    while (p2_agetw(&p2)) {
        fp->label = (unsigned char)(p2.tok.u - TYPETXT + TYPEOPEST);
        fp->label |= p2.tok.u & ~0xff;
        p2_agetw(&p2);
        fp->val = p2.tok.i;
        if (DEBUG)
            printf("fbsetup %d type %o value %x\n", fp->label >> 8, (char)fp->label, fp->val);
        ++fp;
    }
    p2.endtable                  = fp;
    ((struct value *)fp)->type.u = ENDTABFLAG;

    // Do pass 2 (pass 0 of second phase...)
    p2_setup(&p2);
    p2.fin = p2.txtfil;
    p2_assem(&p2);
    if (p2.outmod != 0777)
        p2_aexit(&p2, 1);

    // Now set up for pass 3, including header for a.out
    p2.symtab[0].val.u  = 0;
    p2.symtab[0].type.u = TYPETXT;
    p2.symtab[1].val.u  = 0;
    p2.brtabp           = 0;
    ++p2.passno;
    p2_setup(&p2);
    lseek(p2.fin, 0L, 0);
    p2.hdr.atxtsiz[2] = (p2.hdr.atxtsiz[2] + 1) & ~1;
    p2.hdr.atxtsiz[0] = (p2.hdr.atxtsiz[0] + 1) & ~1;
    p2.hdr.atxtsiz[1] = (p2.hdr.atxtsiz[1] + 1) & ~1;
    p2.datbase        = p2.hdr.atxtsiz[0];
    p2.savdot[1]      = p2.datbase;
    p2.bssbase        = p2.hdr.atxtsiz[0] + p2.hdr.atxtsiz[1];
    p2.savdot[2]      = p2.bssbase;
    p2.symseek        = 2 * p2.hdr.atxtsiz[0] + 2 * p2.hdr.atxtsiz[1] + 020;
    p2.relseek[1]     = 2 * p2.hdr.atxtsiz[0] + p2.hdr.atxtsiz[1] + 020;
    p2.relseek[0]     = p2.hdr.atxtsiz[0] + p2.hdr.atxtsiz[1] + 020;
    p2.aseek[1]       = p2.hdr.atxtsiz[0] + 020;
    p2.aseek[0]       = 020;

    for (p = p2.usymtab; p < sp; ++p)
        p2_doreloc(&p2, p);

    for (fp = p2.fbtab; fp < p2.endtable; ++fp)
        p2_doreloc(&p2, (struct value *)fp);

    p2_oset(&p2, &p2.txtp, 0);
    p2_oset(&p2, &p2.relp, p2.relseek[0]);
    for (i = 8, pi = (int *)&p2.hdr; i > 0; --i, ++pi) {
        p2_aputw(&p2, &p2.txtp, *pi);
    }

    p2_assem(&p2);

    // Flush buffers, append symbol table, close output
    p2_flush(&p2, &p2.txtp);
    p2_flush(&p2, &p2.relp);
    p2.fin = p2.symf;
    lseek(p2.fin, 0L, 0);
    p2_oset(&p2, &p2.txtp, p2.symseek);
    sp = p2.usymtab;
    while (p2_agetw(&p2)) {
        p2_aputw(&p2, &p2.txtp, p2.tok.u);
        p2_agetw(&p2);
        p2_aputw(&p2, &p2.txtp, p2.tok.u);
        p2_agetw(&p2);
        p2_aputw(&p2, &p2.txtp, p2.tok.u);
        p2_agetw(&p2);
        p2_aputw(&p2, &p2.txtp, p2.tok.u);
        p2_aputw(&p2, &p2.txtp, sp->type.u);
        p2_aputw(&p2, &p2.txtp, sp->val.u);
        ++sp;
        p2_agetw(&p2);
        p2_agetw(&p2);
    }
    p2_flush(&p2, &p2.txtp);
    p2_aexit(&p2, 0);
    return 0;
}

//
// Unlink temp files (unless debug) and exit with given code.
// Called on error or normal completion of pass2.
// Inputs: p2 (atmp1, atmp2, atmp3), code (exit status). Uses global debug_flag.
// Outputs: None; process exits.
//
void p2_aexit(struct pass2 *p2, int code)
{
    if (!debug_flag) {
        unlink(p2->atmp1);
        unlink(p2->atmp2);
        unlink(p2->atmp3);
    }
    exit(code);
}

//
// Report file error (name) and exit pass2 with failure.
// Called when a temp or output file cannot be opened.
// Inputs: p2, name (path that failed).
// Outputs: Prints message; calls p2_aexit(p2, 1).
//
void p2_filerr(struct pass2 *p2, char *name)
{
    printf("filerr: File error in file %s\n", name);
    p2_aexit(p2, 1);
}

//
// Apply relocation to a symbol or fb entry: add datbase/bssbase for DATA/BSS, or set defund for UNDEF.
// Called for each usymtab entry and fbtab entry before final assembly pass.
// Inputs: p2 (defund, datbase, bssbase), pv (value to relocate).
// Outputs: pv->type and pv->val updated in place.
//
void p2_doreloc(struct pass2 *p2, struct value *pv)
{
    int t;

    if ((t = pv->type.i) == TYPEUNDEF)
        pv->type.i |= p2->defund;
    t &= 037;
    if (t >= TYPEOPFD || t < TYPEDATA)
        return;
    pv->val.i += (t == TYPEDATA ? p2->datbase : p2->bssbase);
}

//
// Set up for an assembly pass: load opcode table on pass 0, reset nxtfb/curfb and advance fb for 0..9.
// Called at start of each pass (0 and 1) from asm_pass2.
// Inputs: p2 (passno, symtab, fbtab, nxtfb, curfb).
// Outputs: symtab filled from OPTABL on pass 0; nxtfb/curfb initialized; p2_fbadv for each temp index.
//
void p2_setup(struct pass2 *p2)
{
    int i;
    int n;
    FILE *fd;
    struct value *p;
    char dummy[12];

    if (p2->passno == 0) {
        if ((fd = fopen(OPTABL, "r")) == NULL) {
            fprintf(stderr, "setup: can't open %s\n", OPTABL);
            p2_aexit(p2, 1);
        }
        p = &p2->symtab[0];
        while (p - p2->symtab < SYMBOLS &&
               (n = fscanf(fd, "%s %o %o", dummy, &p->type.u, &p->val.u)) == 3) {
            ++p;
        }
        if (p - p2->symtab >= SYMBOLS) {
            fprintf(stderr, "setup: Permanent symbol table overflow\n");
            p2_aexit(p2, 1);
        }
        if (n != -1) {
            fprintf(stderr, "setup: scanned only %d elements after %d symbols\n", n,
                    (int)(p - p2->symtab));
            p2_aexit(p2, 1);
        }
        fclose(fd);
    }

    for (i = 0; i < 10; ++i)
        p2->nxtfb[i] = p2->curfb[i] = 0;
    for (i = 0; i < 10; ++i) {
        p2->tok.i = i;
        p2_fbadv(p2);
    }
}

//
// Open a file for reading; abort pass2 on failure.
// Called to open atmp1, atmp2, atmp3 (and symf) in asm_pass2.
// Inputs: p2 (for p2_filerr), name (path).
// Outputs: Returns open fd; or does not return (p2_filerr then exit).
//
int p2_ofile(struct pass2 *p2, char *name)
{
    int fd;

    if ((fd = open(name, 0)) < 0)
        p2_filerr(p2, name);
    return (fd);
}

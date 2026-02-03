//
// as - PDP/11 Assember, Part 1 - main program
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as1.h"

char atmp1[] = "/tmp/atm1XXXXXX";
char atmp2[] = "/tmp/atm2XXXXXX";
char atmp3[] = "/tmp/atm3XXXXXX";
int debug_flag = 0;

//
// Print assembler usage to stderr and exit with failure.
// Called when invalid options are given or -o has no argument.
//
void usage()
{
    fprintf(stderr, "Usage: asm [-u] [-o outfile] [infile...]\n");
    exit(1);
}

//
// Assembler entry point: parse options, run pass1 then pass2.
// Invoked by the system; -u enables global, -o sets output, -D debug.
// Inputs: argc, argv (optionally -u, -o outfile, input files).
// Outputs: 0 on success; does not return on error or debug early exit.
//
int main(int argc, char *argv[])
{
    int globflag = 0;
    char *outfile = 0;

    while (argv[1] && argv[1][0] == '-') {
        switch (argv[1][1]) {
        case 'u':
            globflag = TRUE;
            break;
        case 'D':
            debug_flag = 1;
            break;
        case 'o':
            outfile = argv[2];
            if (!outfile)
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
    asm_pass1(globflag, argc, argv);
    return asm_pass2(globflag, outfile);
}

//
// Run pass 1: assemble sources into temp files and collect user symbols.
// Called from main after option parsing.
// Inputs: globflag, argc/argv (remaining args as input file list or stdin).
// Outputs: Writes intermediate output to atmp1/atmp2; on error may call aexit.
//
void asm_pass1(int globflag, int argc, char *argv[])
{
    struct pass1 p1;
    memset(&p1, 0, sizeof(p1));
    int fsym;

    pass1_init(&p1);
    p1.globflag = globflag;
    p1.nargs    = argc;
    p1.curarg   = argv;
    p1.pof      = f_create(&p1, atmp1);
    p1.fbfil    = f_create(&p1, atmp2);
    p1.fin      = argc > 1 ? 0 : stdin;
    setup(&p1);
    p1.ch      = 0;
    p1.ifflg   = 0;
    p1.fileflg = 0;
    p1.errflg  = 0;
    p1.savop   = 0;
    assem(&p1);
    close(p1.pof);
    close(p1.fbfil);
    if (p1.errflg)
        aexit(&p1);
    fsym = f_create(&p1, atmp3);
    write_syms(&p1, fsym);
    close(fsym);
}

//
// Write user symbol table to a file descriptor for pass2.
// Called at end of pass1 to dump p1->usymtab into atmp3.
// Inputs: p1 (usymtab, symend), fd (open for writing).
// Outputs: For each user symbol, writes 8-byte name then 4-byte type/val (little-endian).
// Iterates usymtab to symend; packs type and val into 4 bytes each.
//
void write_syms(struct pass1 *p1, int fd)
{
    struct symtab *s;
    char buf[4];

    for (s = p1->usymtab; s < p1->symend; ++s) {
        if (debug_flag)
            printf("--- write atmp3: sym \"%.8s\" type=%o val=%o\n",
                   s->name, s->v.type.u, s->v.val.u);
        write(fd, s->name, sizeof(s->name));
        buf[0] = s->v.type.u;
        buf[1] = s->v.type.u >> 8;
        buf[2] = s->v.val.u;
        buf[3] = s->v.val.u >> 8;
        write(fd, buf, 4);
    }
}

//
// Report a file-related error to stderr (name and message).
// Called when temp file creation or other file ops fail.
// Inputs: p1 (unused), name (file path), msg (error description).
// Outputs: None; prints to stderr and returns.
//
void filerr(struct pass1 *p1, char *name, char *msg)
{
    (void)p1;
    fprintf(stderr, "%s %s\n", name, msg);
    return;
}

//
// Abort without pass2: unlink temp files and exit with failure.
// Called when pass1 hits errors so pass2 is not run.
// Inputs: p1 (unused).
// Outputs: None; process exits with 1.
//
void aexit(struct pass1 *p1)
{
    (void)p1;
    unlink(atmp1);
    unlink(atmp2);
    unlink(atmp3);
    exit(1);
}

//
// Create a temporary file with a unique name; abort on failure.
// Called to create atmp1, atmp2, atmp3 for pass1 intermediates.
// Inputs: p1 (for filerr), name (template e.g. /tmp/atm1XXXXXX).
// Outputs: Open file descriptor (read/write); name is modified in place by mkstemp.
//
int f_create(struct pass1 *p1, char *name)
{
    int fd;

    if ((fd = mkstemp(name)) < 0) {
        filerr(p1, name, "f_create: can't create file.");
        exit(2);
    }
    return (fd);
}

//
// Build the permanent (opcode) symbol table from OPTABL and hash it.
// Called once at start of pass1 after temp files are created.
// Inputs: p1 (symtab, hshtab); OPTABL file must exist.
// Outputs: Fills symtab from file, pads names to 8 chars, enters each in hash via hash_enter.
//
void setup(struct pass1 *p1)
{
    int n;
    struct symtab *p, *e;
    FILE *fd;

    if ((fd = fopen(OPTABL, "r")) == 0) {
        fprintf(stderr, "setup: can't open %s.\n", OPTABL);
        exit(2);
    }
    p = p1->symtab;
    while (p - p1->symtab < SYMBOLS &&
           (n = fscanf(fd, "%s %o %o", p->name, &p->v.type.i, &p->v.val.i)) == 3) {
        ++p;
    }
    if (p - p1->symtab > SYMBOLS) {
        fprintf(stderr, "setup: permanent symbol table overflow.\n");
        exit(2);
    }
    if (n != -1) {
        fprintf(stderr, "setup: scanned only %d elements after %d symbols.\n", n,
                (int)(p - p1->symtab));
        exit(2);
    }
    for (e = p, p = p1->symtab; p < e; ++p) {
        memset(p->name + strlen(p->name), 0, 8 - strlen(p->name));
        hash_enter(p1, p);
    }
    fclose(fd);
}

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

char atmp1[]   = "/tmp/atm1XXXXXX";
char atmp2[]   = "/tmp/atm2XXXXXX";
int debug_flag = 0;

struct symtab  global_symtab[SYMBOLS + USERSYMBOLS];
struct symtab *global_symend;

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
    int globflag  = 0;
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
    assem(&p1);
    close(p1.pof);
    close(p1.fbfil);
    if (p1.errflg)
        aexit(&p1);
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
// Build the permanent (opcode) symbol table from embedded opcode_table and hash it.
// Called once at start of pass1 after temp files are created.
// Inputs: p1 (hshtab).
// Outputs: Fills global_symtab from opcode_table, pads names to 8 chars, enters each in hash.
//
void setup(struct pass1 *p1)
{
    int i;
    struct symtab *p;

    if (opcode_table_size > SYMBOLS) {
        fprintf(stderr, "setup: permanent symbol table overflow.\n");
        exit(2);
    }
    p = global_symtab;
    for (i = 0; i < opcode_table_size; ++i, ++p) {
        memset(p->name, 0, sizeof(p->name));
        strncpy(p->name, opcode_table[i].name, 8);
        p->v.type.u = opcode_table[i].type;
        p->v.val.u  = opcode_table[i].val;
        hash_enter(p1, p);
    }
}

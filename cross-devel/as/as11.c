/*
 * as - PDP/11 Assember, Part 1 - main program
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as1.h"

char atmp1[] = "/tmp/atm1XXXXXX";
char atmp2[] = "/tmp/atm2XXXXXX";
char atmp3[] = "/tmp/atm3XXXXXX";

void usage()
{
    fprintf(stderr, "Usage: asm [-u] [-o outfile] [infile...]\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    int globflag = 0, debug = 0;
    char *outfile = 0;

    while (argv[1] && argv[1][0] == '-') {
        switch (argv[1][1]) {
        case 'u':
            globflag = TRUE;
            break;
        case 'D':
            debug = 1;
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
    if (debug)
        exit(0);
    return asm_pass2(globflag, outfile);
}

void asm_pass1(int globflag, int argc, char *argv[])
{
    struct pass1 p1;
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

void write_syms(struct pass1 *p1, int fd)
{
    struct symtab *s;
    char buf[4];

    for (s = p1->usymtab; s < p1->symend; ++s) {
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
void filerr(struct pass1 *p1, char *name, char *msg)
{
    (void)p1;
    fprintf(stderr, "%s %s\n", name, msg);
    return;
}

/*
        Routine to exit program (without doing pass 2)
*/
void aexit(struct pass1 *p1)
{
    (void)p1;
    unlink(atmp1);
    unlink(atmp2);
    unlink(atmp3);
    exit(1);
}

/*
        Routine to create one of the temporary files
*/
int f_create(struct pass1 *p1, char *name)
{
    int fd;

    if ((fd = mkstemp(name)) < 0) {
        filerr(p1, name, "f_create: can't create file.");
        exit(2);
    }
    return (fd);
}

/*
        Routine to build permanent symbol table
*/
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

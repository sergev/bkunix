/*
 * as - PDP/11 Assembler, Part I
 *
 * Scanner subroutines
 * Symbol table subroutines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as1.h"

/*
        Routine to read a name whose 1st character is
        contained in variable c
*/
void rname(struct pass1 *p1, unsigned char c)
{
    char *sp;
    unsigned char tc;
    unsigned short hv, next;
    struct value *stok;
    int no_hash, sl, probe;
    char debug = 0;

    sl      = 8;
    no_hash = 0;
    memset(p1->symbol, 0, 8);
    sp = p1->symbol;
    if (c == '~') {
        ++no_hash;
        p1->ch = 0;
    }
    while ((c = p1->chartab[tc = rch(p1)]) < 128 && c != 0) {
        if (--sl >= 0)
            *sp++ = c;
    }
    hv     = hash(p1, p1->symbol);
    p1->ch = tc;
    if (no_hash) {
        p1->tok.s = p1->symend;
        add_symbol(p1, p1->tok.s, p1->symbol);
    } else {
        probe = hv % HSHSIZ;
        next  = (hv / HSHSIZ) + 1;
        while (1) {
            if ((probe -= next) < 0)
                probe += HSHSIZ;
            if (debug)
                printf("rname debug probe %d next %u\n", probe, next);
            if (p1->hshtab[probe] == 0) {
                p1->hshtab[probe] = p1->tok.s = p1->symend;
                add_symbol(p1, p1->tok.s, p1->symbol);
                break;
            }
            if (debug)
                printf("rname debug comparing to %s\n", p1->hshtab[probe]->name);
            if (strncmp(p1->hshtab[probe]->name, p1->symbol, 8) == 0) {
                p1->tok.s = p1->hshtab[probe];
                break;
            }
        }
    }

    stok = &p1->tok.s->v;
    if (p1->tok.s >= p1->usymtab)
        p1->tok.i = (p1->tok.s - p1->usymtab) + USYMFLAG;
    else
        p1->tok.i = (p1->tok.s - p1->symtab) + PSYMFLAG;
    aputw(p1);
    p1->tok.v = stok;
}

/*
        Routine to handle numbers and temporary labels
        Numbers starting from 0 are treated as octal.
*/
char number(struct pass1 *p1)
{
    int num, base;
    unsigned char c;

    if ((c = rch(p1)) != '0') {
        base   = 10;
        p1->ch = c;
    } else if ((c = rch(p1)) != 'x' && c != 'X') {
        base   = 8;
        p1->ch = c;
    } else {
        base = 16;
    }
    num = 0;
    for (;;) {
        c = rch(p1);
        if (c >= '0' && c <= '7')
            c -= '0';
        else if (base >= 10 && c >= '8' && c <= '9')
            c -= '0';
        else if (base == 16 && c >= 'a' && c <= 'f')
            c -= 'a' - 10;
        else if (base == 16 && c >= 'A' && c <= 'F')
            c -= 'A' - 10;
        else
            break;
        num = num * base + c;
    }
    if (c != 'b' && c != 'f') {
        p1->num_rtn = num;
        p1->ch      = c;
        return (TRUE);
    }
    /*
            Temporary label reference
    */
    p1->tok.i = fbcheck(p1, num) + (c == 'b' ? FBBASE : FBFWD);
    return (FALSE);
}

/*
        Routine to read next character
        Uses character routines so that MSDOS crlf turns into lf
*/
unsigned char rch(struct pass1 *p1)
{
    int c;
    union token savtok;
    char *pc;

    if ((c = p1->ch) != 0) {
        p1->ch = 0;
        return (c);
    }
    while (TRUE) {
        if (p1->fin != 0) {
            if ((c = fgetc(p1->fin)) != EOF)
                return (c & 0x7f);
        }
        if (p1->fin != 0)
            fclose(p1->fin);
        if (--p1->nargs <= 0)
            return (TOKEOF);
        if (p1->ifflg) {
            aerror(p1, 'i');
            aexit(p1);
        }
        ++p1->fileflg;
        if ((p1->fin = fopen(*++p1->curarg, "r")) == NULL) {
            filerr(p1, *p1->curarg, "rch: can't open file.");
            aexit(p1);
        }
        p1->line  = 1;
        savtok    = p1->tok;
        p1->tok.i = TOKFILE;
        aputw(p1);
        for (pc = *p1->curarg; *pc != '\0'; ++pc) {
            p1->tok.i = *pc;
            aputw(p1);
        }
        p1->tok.i = -1;
        aputw(p1);
        p1->tok = savtok;
    }
}

/*
        Routine to hash a symbol and enter into hash table
*/
void hash_enter(struct pass1 *p1, struct symtab *p)
{
    unsigned short hv, next;
    int probe;
    char debug = 0;

    hv    = hash(p1, p->name);
    probe = hv % HSHSIZ;
    next  = (hv / HSHSIZ) + 1;
    while (TRUE) {
        if ((probe -= next) < 0)
            probe += HSHSIZ;
        if (debug)
            printf("hash_enter: probe %d next %u\n", probe, next);
        if (p1->hshtab[probe] == 0) {
            p1->hshtab[probe] = p;
            break;
        }
    }
}

/*
        Routine to hash a symbol
*/
unsigned short hash(struct pass1 *p1, char *p)
{
    int i;

    unsigned h = 0;
    (void)p1;
    for (i = 0; i < 8 && p[i] != '\0'; ++i) {
        h += p[i];
        h = (h << 8) | ((h >> 8) & 0xFF);
    }
    return (h);
}

/*
        Routine to add a symbol to the symbol table and bump
        the symbol table pointer
*/
void add_symbol(struct pass1 *p1, struct symtab *p, char *s)
{
    strncpy(p->name, s, 8);
    if (++p1->symend - p1->usymtab > USERSYMBOLS) {
        fprintf(stderr, "add_symbol: symbol table overflow.\n");
        aexit(p1);
    }
}

//
// as - PDP/11 Assembler, Part I
//
// Scanner subroutines
// Symbol table subroutines
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as1.h"

//
// Read an identifier starting with c, lookup or create in symbol table, set tok.
// Called from readop when first character of a name is already read (e.g. letter).
// Inputs: p1 (chartab, symbol, hshtab, symend, usymtab); c (first character, or '~' for new symbol).
// Outputs: p1->tok set to symbol token; symbol stored; aputw called to emit token.
// Accumulates name via chartab (max 8 chars), hashes; ~ forces new symbol;
// else linear probe (next = hv/HSHSIZ+1) for lookup/insert.
// Sets tok.i to PSYMFLAG or USYMFLAG offset.
//
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

//
// Parse a number or temporary label (nb/nf); set numval or tok for fb reference.
// Called from readop when a digit is seen; 0-prefix octal, 0x hex, else decimal.
// Inputs: p1 (ch, numval, num_rtn); next char from input.
// Returns TRUE for plain number (num_rtn set, numval for expression); FALSE for nb/nf (tok set to FBBASE/FBFWD index).
// Base 8/10/16 by prefix; accumulate until non-digit.
// Trailing 'b' or 'f' makes temporary label ref via fbcheck.
//
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
    // Temporary label reference
    p1->tok.i = fbcheck(p1, num) + (c == 'b' ? FBBASE : FBFWD);
    return (FALSE);
}

//
// Read next character from current file or advance to next input file; normalize CRLF.
// Used by scanner (readop, rname, number, etc.) to get input; handles #include-like file stacking.
// Inputs: p1 (ch, fin, nargs, curarg, ifflg, fileflg, tok, line).
// Returns next byte (0x7f masked) or TOKEOF; may open next file and emit TOKFILE token.
//
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
        p1->tok.u = TOKFILEND;
        aputw(p1);
        p1->tok = savtok;
    }
}

//
// Insert symbol table pointer p into the hash table (used during setup and rname).
// Called from setup for each opcode symbol and from rname when adding new identifier.
// Inputs: p1 (hshtab), p (symtab entry to register).
// Outputs: p stored in hshtab at probe position; no return.
// Same linear probe as rname (hv % HSHSIZ, next = hv/HSHSIZ+1) until empty slot.
//
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

//
// Compute hash value for an 8-byte (or less) symbol name for hash table indexing.
// Called by rname and hash_enter to get probe start.
// Inputs: p1 (unused), p (name string, null-terminated, max 8 chars considered).
// Outputs: 16-bit hash value.
// Byte-add and rotate: h += p[i]; h = (h<<8)|(h>>8)&0xFF.
//
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

//
// Add a new user symbol: copy name to p, advance symend, check overflow.
// Called from rname when creating a new identifier; p is symend before bump.
// Inputs: p1 (symend, usymtab), p (symtab slot), s (source name string).
// Outputs: p->name filled (strncpy 8); symend incremented; aborts on USERSYMBOLS overflow.
// strncpy(p->name, s, 8); increment symend; aexit if overflow.
//
void add_symbol(struct pass1 *p1, struct symtab *p, char *s)
{
    strncpy(p->name, s, 8);
    if (++p1->symend - p1->usymtab > USERSYMBOLS) {
        fprintf(stderr, "add_symbol: symbol table overflow.\n");
        aexit(p1);
    }
}

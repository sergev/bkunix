//
// AS - PDP/11 Assembler, Part II
//
// Main assembly control routine
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as2.h"

//
// Main pass2 loop: read token stream, handle = and :, dispatch to p2_opline.
// Called twice from asm_pass2 (pass 0 and pass 1 over token file).
// Inputs: p2 (full pass2 state; fin set to txtfil).
// Outputs: Assembly output and relocation written via p2_outw/p2_outb; line count updated.
//
void p2_assem(struct pass2 *p2)
{
    union token ttok;

    while (1) {
        p2_readop(p2);
        if (p2->tok.u != TOKFILE && p2->tok.u != '<') {
            if (p2_checkeos(p2))
                goto ealoop;
            ttok.u = p2->tok.u;
            if (p2->tok.u == TOKINT) {
                ttok.u = 2;
                p2_agetw(p2);
                p2->numval = p2->tok.u;
            }
            p2_readop(p2);
            switch (p2->tok.u) {
            case '=':
                p2_doequal(p2, &ttok);
                goto ealoop;
            case ':':
                p2_docolon(p2, &ttok);
                continue;
            default:
                p2->savop = p2->tok;
                p2->tok.u = ttok.u;
                break;
            }
        }

        p2_opline(p2);
        p2_dotmax(p2);

    ealoop:

        if (p2->tok.u == '\n')
            ++p2->line;
        if (DEBUG)
            printf("\nLine %d: ", p2->line);
        if (p2->tok.u == TOKEOF)
            return;
    }
}

//
// Process label = expression: assign value to symbol or update dot ( .= ).
// Called from p2_assem when token after label is '='.
// Inputs: p2, t (saved token: symbol or dot); next token is expression.
// Outputs: Symbol type/val or dot updated; may call p2_outb for .= padding; p2_aerror on bad type.
//
void p2_doequal(struct pass2 *p2, union token *t)
{
    struct value v;
    int i;

    p2_readop(p2);
    v = p2_express(p2);
    if (t->v == &p2->symtab[0]) { // .=
        v.type.u &= ~TYPEEXT;
        if (v.type.u != dotrel(p2)) {
            p2_aerror(p2, '.');
            return;
        }
        if (v.type.u == TYPEBSS) {
            p2->symtab[0].val.u = v.val.u;
            p2_dotmax(p2);
            return;
        }
        v.val.u -= dot(p2);
        if (v.val.i < 0) {
            p2_aerror(p2, '.');
            return;
        }
        for (i = v.val.i - 1; i >= 0; --i)
            p2_outb(p2, TYPEABS, 0);
        p2_dotmax(p2);
        return;
    }

    if (v.type.u == TYPEEXT)
        p2_aerror(p2, 'r');
    t->v->type.u &= ~037;
    v.type.u &= 037;
    if (v.type.u == TYPEUNDEF)
        v.val.u = 0;
    t->v->type.u |= v.type.u;
    t->v->val.u = v.val.u;
}

//
// Process label definition: set symbol or forward-branch entry to current dot.
// Called from p2_assem when token after label is ':'.
// Inputs: p2, t (saved token: symbol or TOKINT for temp label).
// Outputs: tok restored; symbol type/val or curfb updated; pass 1 checks dot match.
//
void p2_docolon(struct pass2 *p2, union token *t)
{
    unsigned ttype;

    p2->tok.u = t->u;
    if (p2->tok.u < TOKSYMBOL) {
        if (p2->tok.u != 2) {
            p2_aerror(p2, 'x');
            return;
        }
        p2->tok.u = p2->numval;
        p2_fbadv(p2);
        p2->curfb[p2->tok.u]->label &= ~0xff;
        p2->curfb[p2->tok.u]->label |= dotrel(p2);
        p2->brdelt                = p2->curfb[p2->tok.u]->val - dot(p2);
        p2->curfb[p2->tok.u]->val = dot(p2);
        return;
    }

    if (p2->passno == 0) {
        ttype = p2->tok.v->type.u & 037;
        if (ttype != 0 && (ttype < TYPEOPEST || ttype > TYPEOPESD))
            p2_aerror(p2, 'm');
        p2->tok.v->type.u &= ~037;
        p2->tok.v->type.u |= dotrel(p2);
        p2->brdelt       = p2->tok.v->val.u - dot(p2);
        p2->tok.v->val.u = dot(p2);
        return;
    }

    if (dot(p2) != p2->tok.v->val.u)
        p2_aerror(p2, 'p');
    return;
}

//
// Return whether current token ends a statement (newline, semicolon, or EOF).
// Called from p2_assem to decide whether to continue or finish statement.
// Inputs: p2 (tok).
// Outputs: Non-zero if tok is \n, ;, or TOKEOF.
//
int p2_checkeos(struct pass2 *p2)
{
    return (p2->tok.u == '\n' || p2->tok.u == ';' || p2->tok.u == TOKEOF);
}

//
// Advance forward-branch pointer for temp index in p2->tok.i to next entry for that index.
// Called from p2_setup (init) and p2_docolon when defining n:.
// Inputs: p2 (tok.i, curfb, nxtfb, fbbufp).
// Outputs: curfb[tok.i] and nxtfb[tok.i] updated to next matching fb entry.
//
void p2_fbadv(struct pass2 *p2)
{
    struct fb_tab *p;

    p = p2->curfb[p2->tok.i] = p2->nxtfb[p2->tok.i];
    if (p == 0)
        p = p2->fbbufp;
    else
        ++p;
    while ((p->label >> 8) != p2->tok.i && !(p->label & ENDTABFLAG)) {
        ++p;
    }
    if (DEBUG)
        printf("fbadv %ld to %o %o ", (long)p2->tok.i, (char)p->label, p->val);
    p2->nxtfb[p2->tok.i] = p;
}

//
// Update section size in header if current dot exceeds it (pass 0 only).
// Called after each statement in p2_assem to track text/data/bss high water.
// Inputs: p2 (passno, symtab[0], hdr.atxtsiz).
// Outputs: hdr.atxtsiz[dotrel-1] = max(dot, current).
//
void p2_dotmax(struct pass2 *p2)
{
    if (p2->passno == 0 && dot(p2) > p2->hdr.atxtsiz[dotrel(p2) - TYPETXT])
        p2->hdr.atxtsiz[dotrel(p2) - TYPETXT] = dot(p2);
    return;
}

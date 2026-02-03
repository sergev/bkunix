//
// as - PDP/11 Assember, Part I
//
// Main assembly control routine
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as1.h"

//
// Main pass1 loop: read tokens, handle labels/dot/equals, and dispatch to opline.
// Called once from asm_pass1 after setup; drives entire first pass.
// Inputs: p1 (full pass1 state; readop/checkeos/express/opline).
// Outputs: Token stream and fb data written via aputw/write_fb; errflg set on errors.
//
void assem(struct pass1 *p1)
{
    struct value v;
    union token savtok;
    int i;

    while (1) {
        readop(p1);
        if (checkeos(p1))
            goto ealoop;
        if (p1->ifflg) { // Inside .if
            if (p1->tok.u <= TOKSYMBOL)
                continue;
            if (p1->tok.v->type.i == TYPEOPIF)
                ++p1->ifflg;
            if (p1->tok.v->type.i == TYPEOPEIF)
                --p1->ifflg;
            continue;
        }

        savtok = p1->tok;
        readop(p1);
        if (p1->tok.i == '=') {
            readop(p1);
            v = express(p1);
            if (savtok.u < TOKSYMBOL) {
                aerror(p1, 'x');
                goto ealoop;
            }
            if (p1->tok.s == &p1->symtab[0]) {
                v.type.u &= ~TYPEEXT;
                if (v.type.i != dotrel(p1)) {
                    aerror(p1, '.');
                    p1->symtab[0].v.type.i = TYPETXT;
                    goto ealoop;
                }
            }
            savtok.v->type.u &= ~037;
            v.type.u &= 037;
            if (v.type.u == TYPEUNDEF)
                v.val.i = 0;
            savtok.v->type.u |= v.type.u;
            savtok.v->val.i = v.val.i;
            goto ealoop;
        } // =

        if (p1->tok.i == ':') {
            p1->tok = savtok;
            if (p1->tok.u >= TOKSYMBOL) {
                if (p1->tok.v->type.u & 037)
                    aerror(p1, 'm');
                p1->tok.v->type.u |= dotrel(p1);
                p1->tok.v->val.i = dot(p1);
                continue;
            }
            if (p1->tok.i != TOKINT) {
                aerror(p1, 'x');
                continue;
            }
            i               = fbcheck(p1, p1->numval); // n:
            p1->curfbr[i]   = dotrel(p1);
            p1->nxtfb.label = i << 8 | dotrel(p1);
            p1->nxtfb.val   = dot(p1);
            p1->curfb[i]    = dot(p1);
            write_fb(p1, p1->fbfil, &p1->nxtfb);
            continue;
        } // :

        p1->savop = p1->tok.i;
        p1->tok   = savtok;
        opline(p1);

    ealoop:

        if (p1->tok.i == ';')
            continue;
        if (p1->tok.i == '\n') {
            ++p1->line;
            continue;
        }
        if (p1->tok.i != TOKEOF) {
            aerror(p1, 'x');
            while (!checkeos(p1))
                readop(p1);
            continue;
        }
        if (p1->ifflg)
            aerror(p1, 'x');
        return;
    }
}

//
// Write one forward-branch table entry to the fb file for pass2.
// Called from assem when a temporary label (n:) is defined.
// Inputs: p1 (unused), f (fd of fb file), b (label and val to write).
// Outputs: Four bytes (label, label>>8, val, val>>8) written to f.
// Packs struct fb_tab into 4 bytes little-endian; errors to stderr on write failure.
//
void write_fb(struct pass1 *p1, int f, struct fb_tab *b)
{
    char buf[4];

    (void)p1;
    if (debug_flag)
        printf("--- write atmp2: fb label=0x%04x val=0x%04x\n",
               (unsigned)(b->label & 0xffff), (unsigned)(b->val & 0xffff));
    buf[0] = b->label;
    buf[1] = b->label >> 8;
    buf[2] = b->val;
    buf[3] = b->val >> 8;
    if (write(f, buf, 4) != 4)
        fprintf(stderr, "assem: error writing to fb file.\n");
}

//
// Ensure temporary label number is in 0..9; error and clamp otherwise.
// Called when parsing n: or nb/nf temporary label references.
// Inputs: p1 (for aerror), u (candidate label number).
// Outputs: Returns u if 0..9, else 0 after reporting error.
//
unsigned fbcheck(struct pass1 *p1, unsigned u)
{
    if (u > 9) {
        aerror(p1, 'f');
        u = 0;
    }
    return (u);
}

//
// Return whether the current token ends a statement (newline, semicolon, #, or EOF).
// Called from assem loop to decide whether to continue or finish statement.
// Inputs: p1 (tok).
// Outputs: Non-zero if tok is \n, ;, #, or TOKEOF.
//
int checkeos(struct pass1 *p1)
{
    return (p1->tok.i == '\n' || p1->tok.i == ';' || p1->tok.i == '#' || p1->tok.i == TOKEOF);
}

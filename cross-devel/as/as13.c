/*
 * as - PDP/11 Assember, Part I
 *
 * Main assembly control routine
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as1.h"

void assem(struct pass1 *p1)
{
    struct value v;
    union token savtok;
    int i;

    while (1) {
        readop(p1);
        if (checkeos(p1))
            goto ealoop;
        if (p1->ifflg) { /* Inside .if */
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
        } /* = */

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
            i               = fbcheck(p1, p1->numval); /* n: */
            p1->curfbr[i]   = dotrel(p1);
            p1->nxtfb.label = i << 8 | dotrel(p1);
            p1->nxtfb.val   = dot(p1);
            p1->curfb[i]    = dot(p1);
            write_fb(p1, p1->fbfil, &p1->nxtfb);
            continue;
        } /* : */

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

void write_fb(struct pass1 *p1, int f, struct fb_tab *b)
{
    char buf[4];

    (void)p1;
    buf[0] = b->label;
    buf[1] = b->label >> 8;
    buf[2] = b->val;
    buf[3] = b->val >> 8;
    if (write(f, buf, 4) != 4)
        fprintf(stderr, "assem: error writing to fb file.\n");
}

/*
        Routine to check a number to see if it is in range for
        a temporary label
*/
unsigned fbcheck(struct pass1 *p1, unsigned u)
{
    if (u > 9) {
        aerror(p1, 'f');
        u = 0;
    }
    return (u);
}

/*
        Routine to check current token to see if we are at the end of
        a statement
*/
int checkeos(struct pass1 *p1)
{
    return (p1->tok.i == '\n' || p1->tok.i == ';' || p1->tok.i == '#' || p1->tok.i == TOKEOF);
}

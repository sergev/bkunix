//
// AS - PDP/11 Assembler, Part II
//
// Expression parsing / evaluation.
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as2.h"

//
// Top-level expression entry: clear xsymbol and evaluate.
// Called from p2_opline and p2_doequal when an expression is needed.
// Inputs: p2 (tok, passno, numval, curfb, usymtab, symtab).
// Outputs: Returns struct value (type and val); consumes tokens.
//
struct value p2_express(struct pass2 *p2)
{
    p2->xsymbol = 0;
    return (p2_expres1(p2));
}

//
// Parse and evaluate expression with + - * / & | << >> % ^ ! ; operands from symbol, TOKINT, 2,
// [...]. Called by p2_express; recursive for subexpressions in brackets. Inputs: p2 (tok, passno,
// numval, curfb, symtab, usymtab); p2_combine, pass2_relt* tables. Outputs: Returns accumulated
// value; sets p2->xsymbol for TYPEEXT operands.
//
struct value p2_expres1(struct pass2 *p2)
{
    struct value v, rv;
    int oldop;
    struct fb_tab *pfb;

    oldop    = '+';
    v.val.i  = 0;
    v.type.i = TYPEABS;

    while (1) {
        if (p2->tok.i > TOKSYMBOL) {
            if ((rv.type.i = p2->tok.v->type.i) == TYPEUNDEF && p2->passno != 0)
                p2_aerror(p2, "Unknown symbol");
            if (rv.type.i == TYPEEXT) {
                p2->xsymbol = (void *)(size_t)p2->tok.u;
                rv.val.i    = 0;
                goto operand;
            }
            rv.val.i = p2->tok.v->val.i;
            goto operand;
        }

        // Temp labels only: 0b-9b (FBBASE..FBBASE+9), 0f-9f (FBFWD..FBFWD+9). Symbol tokens
        // must not match here (they are > TOKSYMBOL or hold pointer after resolution).
        if (p2->tok.u >= FBBASE && p2->tok.u < FBFWD + 10) {
            pfb       = p2->curfb[p2->tok.u - FBBASE];
            rv.val.i  = pfb->val;
            rv.type.i = (char)pfb->label;
            goto operand;
        }

        switch (p2->tok.u) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '&':
        case '%':
        case '^':
        case '!':
        case TOKVBAR:
        case TOKLSH:
        case TOKRSH:
            if (oldop != '+')
                p2_aerror(p2, "Bad expression");
            oldop = p2->tok.u;
            p2_readop(p2);
            continue;

        case TOKINT:
            p2_agetw(p2);
            rv.val.i  = p2->tok.i;
            rv.type.i = TYPEABS;
            goto operand;

        case 2:
            rv.val.i  = p2->numval;
            rv.type.i = TYPEABS;
            goto operand;

        case '[':
            p2_readop(p2);
            rv = p2_expres1(p2);
            if (p2->tok.u != ']')
                p2_aerror(p2, "Required ']'");
            goto operand;

        default:
            return (v);
        }

    operand:

        switch (oldop) {
        case '+':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_reltp2(p2));
            v.val.i += rv.val.i;
            break;

        case '-':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_reltm2(p2));
            v.val.i -= rv.val.i;
            break;

        case '*':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i *= rv.val.i;
            break;

        case '/':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i /= rv.val.i;
            break;

        case TOKVBAR:
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i |= rv.val.i;
            break;

        case '&':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i &= rv.val.i;
            break;

        case TOKLSH:
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i <<= rv.val.i;
            break;

        case TOKRSH:
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.u >>= rv.val.i;
            break;

        case '%':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i %= rv.val.i;
            break;

        case '^':
            v.type.i = rv.type.i;
            break;

        case '!':
            v.type.i = p2_combine(p2, v.type.i, rv.type.i, pass2_relte2(p2));
            v.val.i += ~rv.val.u;
            break;

        default:
            break;
        }

        oldop = '+';
        p2_readop(p2);
    }
    return (v); // never execued - for compiler
}

//
// Compute result type when combining two operand types using the given table (add/sub/mul-style).
// Called from p2_expres1 for each binary operator; table is reltp2, reltm2, or relte2.
// Inputs: p2 (passno, maxtyp), left/right (types), table (6x6 int array).
// Outputs: Returns combined type; pass 1 uses table lookup and updates maxtyp.
//
int p2_combine(struct pass2 *p2, int left, int right, int *table)
{
    int t, t2;

    if (p2->passno == 0) {
        t = (right | left) & TYPEEXT;
        right &= 037;
        left &= 037;
        if (right > left) {
            t2    = right;
            right = left;
            left  = t2;
        }
        if (right == TYPEUNDEF)
            return (t);
        if (table != pass2_reltm2(p2) || left != right)
            return (t | left);
        return (t | TYPEABS);
    }

    p2->maxtyp = 0;
    left       = table[p2_maprel(p2, right) * 6 + p2_maprel(p2, left)];
    if (left < 0) {
        if (left != -1)
            p2_aerror(p2, "Relocatable value not allowed");
        return (p2->maxtyp);
    }
    return (left);
}

//
// Map type to relocation table index (0..5) and update maxtyp for pass 1.
// Called from p2_combine to index relt*2 tables and track maximum type.
// Inputs: p2 (maxtyp), type (TYPEEXT, TYPEUNDEF, section, etc.).
// Outputs: Returns index 0..5 (EXT→5, else type&037); maxtyp = max(maxtyp, type).
//
int p2_maprel(struct pass2 *p2, int type)
{
    if (type == TYPEEXT)
        return (5);
    if ((type &= 037) > p2->maxtyp)
        p2->maxtyp = type;
    if (type > TYPEOPFD)
        return (1);
    return (type);
}

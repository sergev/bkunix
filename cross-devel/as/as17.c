//
// as - PDP/11 assembler, Part I
//
// expression evaluator
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include "as.h"
#include "as1.h"

//
// Parse and evaluate a single expression; support + - * / & | << >> % ^ ! and operands.
// Called from opline, address, assem for operands and .if expressions.
// Inputs: p1 (tok, numval, symtab, curfb/curfbr, usymtab/symend); readop, combine.
// Outputs: Returns struct value (type and val); consumes tokens.
//
struct value express(struct pass1 *p1)
{
    struct value v, rv;
    int opfound, ttype;
    char oldop;

    oldop    = '+';
    opfound  = 0;
    v.val.i  = 0;
    v.type.i = TYPEABS;

    while (1) {
        if (p1->tok.v >= &global_symtab[0].v && p1->tok.v < &global_symend[0].v) { // name/opcode
            rv.type.i = p1->tok.v->type.i;
            rv.val.i  = p1->tok.v->val.i;
            goto operand;
        }
        if (p1->tok.u >= FBBASE) { // local symbol reference
            if (p1->tok.u >= FBFWD) {
                v.type.i = TYPEUNDEF;
                v.val.i  = 0;
                goto operand;
            }
            rv.type.i = p1->curfbr[p1->tok.i - FBBASE];
            rv.val.i  = p1->curfb[p1->tok.i - FBBASE];
            if (v.val.i < 0)
                aerror(p1, "Invalid temporary label");
            goto operand;
        }

        switch (p1->tok.i) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '&':
        case TOKVBAR:
        case TOKLSH:
        case TOKRSH:
        case '%':
        case '^':
        case '!':
            if (oldop != '+')
                aerror(p1, "Invalid expression");
            oldop = p1->tok.u;
            readop(p1);
            continue;

        case TOKINT:
            rv.val.i  = p1->numval;
            rv.type.i = TYPEABS;
            break;

        case '[':
            readop(p1);
            rv = express(p1);
            if (p1->tok.u != ']')
                aerror(p1, "Expected ']'");
            break;

        default:
            if (!opfound)
                aerror(p1, "Invalid expression");
            return (v);
        }

    operand:

        ++opfound;
        ttype = combine(p1, v.type.i, rv.type.i, 0); // tentative
        switch (oldop) {
        case '+':
            v.type.i = ttype;
            v.val.i += rv.val.i;
            break;

        case '-':
            v.type.i = combine(p1, v.type.i, rv.type.i, 1);
            v.val.i -= rv.val.i;
            break;

        case '*':
            v.type.i = ttype;
            v.val.i *= rv.val.i;
            break;

        case '/':
            v.type.i = ttype;
            v.val.i /= rv.val.i;
            break;

        case TOKVBAR:
            v.type.i = ttype;
            v.val.i |= rv.val.i;
            break;

        case '&':
            v.type.i = ttype;
            v.val.i &= rv.val.i;
            break;

        case TOKLSH:
            v.type.i = ttype;
            v.val.i <<= rv.val.i;
            break;

        case TOKRSH:
            v.type.i = ttype;
            v.val.u >>= rv.val.i;
            break;

        case '%':
            v.type.i = ttype;
            v.val.i %= rv.val.i;
            break;

        case '!':
            v.type.i = ttype;
            v.val.i += ~rv.val.u;
            break;

        case '^':
            v.type.i = rv.type.i;
            break;

        default:
            break;
        }

        oldop = '+';
        readop(p1);
    }
    return (v); // dummy...
}

//
// Compute result type when combining two operand types (add/subtract rules for relocation).
// Called from express when applying + or - to get resultant type.
// Inputs: p1 (unused), left/right (type values), sflag (1 for subtraction).
// Outputs: Returns combined type (TYPEEXT preserved; UNDEF propagates; subtract same type → ABS).
//
int combine(struct pass1 *p1, int left, int right, int sflag)
{
    int ext, t;

    (void)p1;
    ext = (left | right) & TYPEEXT;
    left &= 037;
    right &= 037;
    if (right > left) { // highest type on left
        t     = right;
        right = left;
        left  = t;
    }
    if (right == TYPEUNDEF) // if either one was undef
        return (ext);
    if (!sflag) // not subtract
        return (left | ext);
    if (left != right) // subtract unlike types
        return (left | ext);
    return (ext | TYPEABS); // subtract like types
}

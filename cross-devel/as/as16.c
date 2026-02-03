//
// as - PDP/11 assembler, Part I
//
// Statement (Opcode/Operand) Handling and addressing mode parsing
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the MIT License.
// See the accompanying file "LICENSE" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as1.h"

//
// Process one statement: expression, pseudo-op, or instruction; update location counter.
// Called from assem when current token is not '=' or ':' (i.e. after label/assignment).
// Inputs: p1 (tok, symtab[0].v for dot/type, numval, ifflg); readop/express/address.
// Outputs: symtab[0].v.val.u (and type) updated for bytes to emit; may call aerror/aexit.
//
void opline(struct pass1 *p1)
{
    struct value v;
    int t;

    if (p1->tok.u <= TOKSYMBOL) { // Operator
        if (p1->tok.u != '<') {
            express(p1);
            p1->symtab[0].v.val.u += 2;
            return;
        }
        p1->symtab[0].v.val.u += p1->numval; // <string>
        readop(p1);
        return;
    }

    t = p1->tok.v->type.u;
    if (t == TYPEREGIS || t < TYPEOPFD || t > TYPEOPJCC) { // not op code
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;
    }

    readop(p1);
    switch (t) {
    case TYPEOPBR: // 1 word instructions
    case TYPEOPRTS:
    case TYPEOPSYS:
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPJSR: // 2 operand instructions
    case TYPEOPDO:
    case TYPEOPMOVF:
    case TYPEOPFF:
    case TYPEOPFD:
    case TYPEOPMUL:
        address(p1);
        if (p1->tok.i != ',') {
            aerror(p1, "Invalid register name");
            return;
        }
        readop(p1);
        address(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPSO: // 1 operand instructions
        address(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPBYTE: // .byte
        while (1) {
            express(p1);
            ++p1->symtab[0].v.val.u;
            if (p1->tok.i != ',')
                break;
            readop(p1);
        }
        return;

    case TYPEOPWORD: // .word
        while (1) {
            express(p1);
            p1->symtab[0].v.val.u += 2;
            if (p1->tok.i != ',')
                break;
            readop(p1);
        }
        return;

    case TYPEOPASC: // <...>
        p1->symtab[0].v.val.u += p1->numval;
        readop(p1);
        return;

    case TYPEOPEVEN: // .even
        p1->symtab[0].v.val.u = (p1->symtab[0].v.val.u + 1) & ~1;
        return;

    case TYPEOPIF: // .if
        v = express(p1);
        if (v.type.i == TYPEUNDEF)
            aerror(p1, "Undefined identifier in .if");
        if (v.val.i == 0)
            ++p1->ifflg;
        return;

    case TYPEOPEIF: // .endif
        return;

    case TYPEOPGLB: // .globl
        while (p1->tok.v >= &p1->symtab[0].v && p1->tok.v < &p1->symend->v) {
            p1->tok.v->type.u |= TYPEEXT;
            readop(p1);
            if (p1->tok.u != ',')
                break;
            readop(p1);
        }
        return;

    case TYPEREGIS:
    case TYPEOPEST:
    case TYPEOPESD:
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPTXT: // .txt, .data, .bss
    case TYPEOPDAT:
    case TYPEOPBSS:
        p1->savdot[dotrel(p1) - TYPETXT] = dot(p1);
        p1->symtab[0].v.val.u            = p1->savdot[t - TYPEOPTXT];
        p1->symtab[0].v.type.i           = t - TYPEOPTXT + TYPETXT;
        return;

    case TYPEOPSOB: // sob
        express(p1);
        if (p1->tok.u != ',')
            aerror(p1, "Invalid register name");
        readop(p1);
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPCOM: // .common
        if (p1->tok.u < TOKSYMBOL) {
            aerror(p1, "Syntax error");
            return;
        }
        p1->tok.v->type.u |= TYPEEXT;
        readop(p1);
        if (p1->tok.u != ',') {
            aerror(p1, "Syntax error");
            return;
        }
        readop(p1);
        express(p1);
        return;

    case TYPEOPJBR: // jbr
        v = express(p1);
        if (v.type.i != dotrel(p1) || (v.val.i -= dot(p1)) > 0 || v.val.i < -376)
            p1->symtab[0].v.val.u += 4;
        else
            p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPJCC: // jcc
        v = express(p1);
        if (v.type.i != dotrel(p1) || (v.val.i -= dot(p1)) > 0 || v.val.i < 376)
            p1->symtab[0].v.val.u += 6;
        else
            p1->symtab[0].v.val.u += 2;
        return;
    default:
        break;
    }

    aerror(p1, "Internal error");
    fprintf(stderr, "opline: internal error, line %d\n", p1->line);
    aexit(p1);
}

//
// Parse one addressing mode (register, (reg), -(reg), $imm, *expr, expr, expr(reg)); update dot.
// Called from opline for instruction operands.
// Inputs: p1 (tok, savop, symtab[0]); readop, express, checkrp, checkreg.
// Outputs: Returns extra words for dot (0 or 2); symtab[0].v.val.u updated; consumes tokens.
//
int address(struct pass1 *p1)
{
    int i;
    struct value v;

    switch (p1->tok.i) {
    case '(':
        readop(p1);
        v = express(p1);
        checkrp(p1);
        checkreg(p1, v);
        if (p1->tok.i == '+') {
            readop(p1);
            return (0);
        }
        return (2);

    case '-':
        readop(p1);
        if (p1->tok.i != '(') { // not really auto decrement
            p1->savop = p1->tok;
            p1->tok.i = '-';
            break;
        }
        readop(p1);
        v = express(p1);
        checkrp(p1);
        checkreg(p1, v);
        return (0);

    case '$':
        readop(p1);
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return (0);

    case '*':
        readop(p1);
        if (p1->tok.i == '*')
            aerror(p1, "Error at '*'");
        i = address(p1);
        p1->symtab[0].v.val.u += i;
        return (i);

    default:
        break;
    }

    v = express(p1);
    if (p1->tok.i == '(') { // indexed
        readop(p1);
        v = express(p1);
        checkreg(p1, v);
        checkrp(p1);
        p1->symtab[0].v.val.u += 2;
        return (0);
    }
    if (v.type.i == TYPEREGIS) {
        checkreg(p1, v);
        return (0);
    }
    p1->symtab[0].v.val.u += 2;
    return (0);
}

//
// Verify value is a valid register (0..7) and not a section type; error otherwise.
// Called from address after parsing (reg) or register operand.
// Inputs: p1 (for aerror), v (value to check).
// Outputs: None; calls aerror(p1,"Invalid register name") if v.val.u>7 or v.type is section-like.
//
void checkreg(struct pass1 *p1, struct value v)
{
    if (v.val.u > 7 || (v.type.u != TYPEABS && v.type.u <= TYPEBSS))
        aerror(p1, "Invalid register name");
}

//
// Require current token to be ')' and advance to next token.
// Called after parsing expression inside parentheses in address.
// Inputs: p1 (tok).
// Outputs: If tok is ')', readop; else aerror and return without advancing.
//
void checkrp(struct pass1 *p1)
{
    if (p1->tok.i != ')') {
        aerror(p1, "Expected ')'");
        return;
    }
    readop(p1);
}

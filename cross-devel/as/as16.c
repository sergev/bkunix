/*
 * as - PDP/11 assembler, Part I
 *
 * Statement (Opcode/Operand) Handling and addressing mode parsing
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as1.h"

/*
        Routine to process one statement
*/
void opline(struct pass1 *p1)
{
    struct value v;
    int t;

    if (p1->tok.u <= TOKSYMBOL) { /* Operator */
        if (p1->tok.u != '<') {
            express(p1);
            p1->symtab[0].v.val.u += 2;
            return;
        }
        p1->symtab[0].v.val.u += p1->numval; /* <string> */
        readop(p1);
        return;
    }

    t = p1->tok.v->type.u;
    if (t == TYPEREGIS || t < TYPEOPFD || t > TYPEOPJCC) { /* not op code */
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;
    }

    readop(p1);
    switch (t) {
    case TYPEOPBR: /* 1 word instructions */
    case TYPEOPRTS:
    case TYPEOPSYS:
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPJSR: /* 2 operand instructions */
    case TYPEOPDO:
    case TYPEOPMOVF:
    case TYPEOPFF:
    case TYPEOPFD:
    case TYPEOPMUL:
        address(p1);
        if (p1->tok.i != ',') {
            aerror(p1, 'a');
            return;
        }
        readop(p1);
        address(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPSO: /* 1 operand instructions */
        address(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPBYTE: /* .byte	*/
        while (1) {
            express(p1);
            ++p1->symtab[0].v.val.u;
            if (p1->tok.i != ',')
                break;
            readop(p1);
        }
        return;

    case TYPEOPWORD: /* .word	*/
        while (1) {
            express(p1);
            p1->symtab[0].v.val.u += 2;
            if (p1->tok.i != ',')
                break;
            readop(p1);
        }
        return;

    case TYPEOPASC: /* <...>	*/
        p1->symtab[0].v.val.u += p1->numval;
        readop(p1);
        return;

    case TYPEOPEVEN: /* .even	*/
        p1->symtab[0].v.val.u = (p1->symtab[0].v.val.u + 1) & ~1;
        return;

    case TYPEOPIF: /* .if		*/
        v = express(p1);
        if (v.type.i == TYPEUNDEF)
            aerror(p1, 'U');
        if (v.val.i == 0)
            ++p1->ifflg;
        return;

    case TYPEOPEIF: /* .endif	*/
        return;

    case TYPEOPGLB: /* .globl	*/
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

    case TYPEOPTXT: /* .txt, .data, .bss */
    case TYPEOPDAT:
    case TYPEOPBSS:
        p1->savdot[dotrel(p1) - TYPETXT] = dot(p1);
        p1->symtab[0].v.val.u            = p1->savdot[t - TYPEOPTXT];
        p1->symtab[0].v.type.i           = t - TYPEOPTXT + TYPETXT;
        return;

    case TYPEOPSOB: /* sob		*/
        express(p1);
        if (p1->tok.u != ',')
            aerror(p1, 'a');
        readop(p1);
        express(p1);
        p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPCOM: /* .common	*/
        if (p1->tok.u < TOKSYMBOL) {
            aerror(p1, 'x');
            return;
        }
        p1->tok.v->type.u |= TYPEEXT;
        readop(p1);
        if (p1->tok.u != ',') {
            aerror(p1, 'x');
            return;
        }
        readop(p1);
        express(p1);
        return;

    case TYPEOPJBR: /* jbr		*/
        v = express(p1);
        if (v.type.i != dotrel(p1) || (v.val.i -= dot(p1)) > 0 || v.val.i < -376)
            p1->symtab[0].v.val.u += 4;
        else
            p1->symtab[0].v.val.u += 2;
        return;

    case TYPEOPJCC: /* jcc		*/
        v = express(p1);
        if (v.type.i != dotrel(p1) || (v.val.i -= dot(p1)) > 0 || v.val.i < 376)
            p1->symtab[0].v.val.u += 6;
        else
            p1->symtab[0].v.val.u += 2;
        return;
    default:
        break;
    }

    aerror(p1, '~');
    fprintf(stderr, "opline: internal error, line %d\n", p1->line);
    aexit(p1);
}

/*
        Routine to parse an address and return bytes needed
*/
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
        if (p1->tok.i != '(') { /* not really auto decrement */
            p1->savop = p1->tok.i;
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
            aerror(p1, '*');
        i = address(p1);
        p1->symtab[0].v.val.u += i;
        return (i);

    default:
        break;
    }

    v = express(p1);
    if (p1->tok.i == '(') { /* indexed */
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

/*
        Routine to check that a value is in range for a register
*/
void checkreg(struct pass1 *p1, struct value v)
{
    if (v.val.u > 7 || (v.type.u != TYPEABS && v.type.u <= TYPEBSS))
        aerror(p1, 'a');
}

/*
        Routine to check for an expected right paren
*/
void checkrp(struct pass1 *p1)
{
    if (p1->tok.i != ')') {
        aerror(p1, ')');
        return;
    }
    readop(p1);
}

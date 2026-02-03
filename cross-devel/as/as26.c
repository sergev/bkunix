//
// AS - PDP/11 Assembler, Part II
//
// opline - statment processing
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as2.h"

//
// Process one statement: expression, pseudo-op, or instruction; emit words/bytes and update dot.
// Called from p2_assem for each statement (after = and : handling).
// Inputs: p2 (tok, symtab, padrb, adrbuf, rlimit, swapf, passno, brtab); token stream.
// Outputs: p2_outw/p2_outb; symtab[0] and section state updated; TOKFILE fills argb.
//
void p2_opline(struct pass2 *p2)
{
    struct value v;
    unsigned topcode, ttype, tb, a1;
    char *pf;
    struct value *ttok;

    // Handle non symbol tokens
    if (p2->tok.u < TOKSYMBOL) {
        if (p2->tok.u == TOKFILE) {
            p2->line = 1;
            for (pf = p2->argb; p2_agetw(p2) && p2->tok.u != TOKFILEND; ++pf) {
                *pf = p2->tok.u;
            }
            *pf = '\0';
            return;
        }
        if (p2->tok.u != '<') {
            v = p2_express(p2);
            p2_outw(p2, v.type.i, v.val.i);
            return;
        }
        goto opl17;
    }

    // Handle non-opcode symbols
    tb = p2->tok.v->type.u;
    if (tb == TYPEREGIS || tb == TYPEOPEST || tb == TYPEOPESD || tb < TYPEOPFD || tb > TYPEOPJCC) {
        v = p2_express(p2);
        p2_outw(p2, v.type.i, v.val.i);
        return;
    }

    // Handle op codes
    topcode = p2->tok.v->val.u;
    ttype   = tb;
    p2_readop(p2);
    p2->padrb  = &p2->adrbuf[0];
    p2->swapf  = 0;
    p2->rlimit = -1;

    switch (ttype) {
    case TYPEOPFD:
        p2->rlimit = 0400;
        p2_op2a(p2, topcode);
        return;

    case TYPEOPBR:
        v = p2_express(p2);

    dobranch:

        if (p2->passno != 0) {
            v.val.i -= dot(p2);
            if (v.val.i < -254 || v.val.i > 256 || (v.val.u & 1) || v.type.u != dotrel(p2)) {
                if (DEBUG)
                    printf(" branch error val %d type %d dotrel %d ", v.val.i, v.type.u,
                           dotrel(p2));
                p2_aerror(p2, 'b');
                v.val.u = 0;
            } else {
                v.val.u = ((v.val.u >> 1) - 1) & 0377;
            }
        }
        v.val.u |= topcode;
        p2_outw(p2, 0, v.val.u);
        return;

    case TYPEOPJSR:
        v = p2_express(p2);
        p2_checkreg(p2, &v);
        p2_readop(p2);
        p2_op2b(p2, v.val.u, topcode);
        return;

    case TYPEOPRTS:
        v = p2_express(p2);
        p2_checkreg(p2, &v);
        v.val.u |= topcode;
        p2_outw(p2, v.type.u, v.val.u);
        return;

    case TYPEOPSYS:
        v = p2_express(p2);
        if (v.val.u >= 64 || v.type.u > TYPEABS)
            p2_aerror(p2, 'a');
        v.val.u |= topcode;
        p2_outw(p2, v.type.u, v.val.u);
        return;

    case TYPEOPMOVF:
        p2->rlimit = 0400;
        a1         = p2_address(p2);
        if (a1 >= 4) {
            ++p2->swapf;
            p2_readop(p2);
            p2_op2b(p2, a1, topcode);
            return;
        }
        p2_readop(p2);
        p2_op2b(p2, a1, 0174000);
        return;

    case TYPEOPDO:
        p2_op2a(p2, topcode);
        return;

    case TYPEOPFF:
        ++p2->swapf;
        p2->rlimit = 0400;
        p2_op2a(p2, topcode);
        return;

    case TYPEOPSO:
        p2_op2b(p2, 0, topcode);
        return;

    case TYPEOPBYTE:
        do {
            v = p2_express(p2);
            p2_outb(p2, v.type.u, v.val.u);
        } while (p2->tok.u == ',' && (p2_readop(p2), 1));
        return;

    case TYPEOPWORD:
        do {
            v = p2_express(p2);
            p2_outw(p2, v.type.u, v.val.u);
        } while (p2->tok.u == ',' && (p2_readop(p2), 1));
        return;

    case TYPEOPASC:
    opl17:
        p2_agetw(p2);
        while ((short)p2->tok.i >= 0) {
            p2_outb(p2, TYPEABS, p2->tok.u & 0377);
            p2_agetw(p2);
        }
        p2_agetw(p2);
        return;

    case TYPEOPEVEN:
        if ((dot(p2) & 1) == 0)
            return;
        if (dotrel(p2) == TYPEBSS) {
            p2->symtab[0].val.u++;
            return;
        }
        p2_outb(p2, 0, 0);
        return;

    case TYPEOPIF:
        p2_express(p2);
        return;

    case TYPEOPEIF:
        return;

    case TYPEOPGLB:
        while (p2->tok.u >= TOKSYMBOL) {
            p2->tok.v->type.u |= TYPEEXT;
            p2_readop(p2);
            if (p2->tok.u != ',')
                break;
            p2_readop(p2);
        }
        return;

    case TYPEREGIS:
    case TYPEOPEST:
    case TYPEOPESD:
        v = p2_express(p2);
        p2_outw(p2, v.type.u, v.val.u);
        return;

    case TYPEOPTXT:
    case TYPEOPDAT:
    case TYPEOPBSS:
        p2->symtab[0].val.u              = (p2->symtab[0].val.u + 1) & ~1;
        p2->savdot[dotrel(p2) - TYPETXT] = dot(p2);
        if (p2->passno != 0) {
            p2_flush(p2, &p2->txtp);
            p2_flush(p2, &p2->relp);
            p2->tseekp = &p2->aseek[ttype - TYPEOPTXT];
            p2_oset(p2, &p2->txtp, *p2->tseekp);
            p2->rseekp = &p2->relseek[ttype - TYPEOPTXT];
            p2_oset(p2, &p2->relp, *p2->rseekp);
        }
        p2->symtab[0].val.u  = p2->savdot[ttype - TYPEOPTXT];
        p2->symtab[0].type.u = ttype - TYPEOPTXT + TYPETXT;
        return;

    case TYPEOPMUL:
        ++p2->swapf;
        p2->rlimit = 01000;
        p2_op2a(p2, topcode);
        return;

    case TYPEOPSOB:
        v = p2_express(p2);
        p2_checkreg(p2, &v);
        tb = v.val.u;
        tb = (tb << 8) | ((tb >> 8) & 0xff);
        topcode |= (tb >> 2);
        p2_readop(p2);
        v = p2_express(p2);
        if (p2->passno != 0) {
            v.val.u -= dot(p2);
            v.val.i = -v.val.i;
            if (v.val.i < -2 || v.val.i > 0175) {
                p2_aerror(p2, 'b');
                p2_outw(p2, 0, topcode);
                return;
            }
            v.val.u += 4;
            if ((v.val.u & 1) || v.type.u != dotrel(p2)) {
                p2_aerror(p2, 'b');
                p2_outw(p2, 0, topcode);
                return;
            }
            v.val.u = ((v.val.u >> 1) - 1) & 0377;
        }
        v.val.u |= topcode;
        p2_outw(p2, 0, v.val.u);
        return;

    case TYPEOPCOM:
        if (p2->tok.u >= TOKSYMBOL) {
            ttok = p2->tok.v;
            p2_readop(p2);
            p2_readop(p2);
            v = p2_express(p2);
            if ((ttok->type.u & 037) == TYPEUNDEF) {
                ttok->type.u |= TYPEEXT;
                ttok->val.u = v.val.u;
            }
        }
        return;

    case TYPEOPJBR:
    case TYPEOPJCC:
        v = p2_express(p2);
        if (p2->passno == 0) {
            v.val.u = p2_setbr(p2, v.val.u);
            if (v.val.u != 0 && topcode != OPCODBR)
                v.val.u += 2;
            p2->symtab[0].val.u += v.val.u + 2;
            return;
        }
        if (p2_getbr(p2) == 0)
            goto dobranch;
        if (topcode != OPCODBR)
            p2_outw(p2, TYPEABS, topcode ^ 0402);
        p2_outw(p2, TYPEABS, OPCODJMP + 037);
        p2_outw(p2, v.type.u, v.val.u);
        return;

    default:
        v = p2_express(p2);
        p2_outw(p2, v.type.u, v.val.u);
        return;
    }
}

//
// Parse first operand of two-operand instruction, then second, and emit combined word plus deferred
// words. Called for TYPEOPFD, TYPEOPDO, TYPEOPFF, TYPEOPMUL style instructions. Inputs: p2 (padrb,
// adrbuf, rlimit, swapf), op (opcode bits). Outputs: p2_address for both operands; p2_op2b emits
// instruction word and any padrb entries.
//
void p2_op2a(struct pass2 *p2, unsigned op)
{
    unsigned a1;

    a1 = p2_address(p2);
    p2_readop(p2);
    p2_op2b(p2, a1, op);
}

//
// Emit second (or only) operand word: combine a1/a2 into PDP-11 mode word, then emit deferred words
// from adrbuf. Called from p2_op2a and for single second operand (e.g. JSR); swapf may swap a1/a2.
// Inputs: p2 (padrb, adrbuf, rlimit, swapf, xsymbol), a1 (first mode), op (opcode), second operand
// from p2_address. Outputs: One instruction word via p2_outw(0, a2); then p2_outw for each adrbuf
// triple (val, type, xsymbol).
//
void p2_op2b(struct pass2 *p2, unsigned a1, unsigned op)
{
    unsigned a2, t;
    unsigned *p;

    a2 = p2_address(p2);
    if (p2->swapf) {
        t  = a1;
        a1 = a2;
        a2 = t;
    }
    a1 = (a1 << 8) | ((a1 >> 8) & 0xff);
    a1 >>= 2;
    if (a1 >= (unsigned)p2->rlimit)
        p2_aerror(p2, 'x');
    a2 |= a1 | op;
    p2_outw(p2, 0, a2);
    for (p = &p2->adrbuf[0]; p < p2->padrb; p += 3) {
        p2->xsymbol = (void *)(size_t)p[2];
        p2_outw(p2, p[1], p[0]);
    }
    return;
}

//
// Parse one addressing mode; return PDP-11 mode byte; deferred values appended to padrb.
// Called from p2_opline/p2_op2a/p2_op2b for instruction operands.
// Inputs: p2 (tok, padrb, adrbuf, xsymbol); readop/express/checkrp/checkreg.
// Outputs: Returns mode value (register, (reg), -(reg), $imm, *expr, expr, expr(reg)); padrb filled
// for deferred/imm/indexed.
//
unsigned p2_address(struct pass2 *p2)
{
    struct value v;
    int t;

    t = 0;
    while (1) {
        switch (p2->tok.u) {
        case '(':
            p2_readop(p2);
            v = p2_express(p2);
            p2_checkrp(p2);
            p2_checkreg(p2, &v);
            if (p2->tok.u == '+') {
                p2_readop(p2);
                v.val.u |= AMAUTOINCR | t;
                return (v.val.u);
            }
            if (t == 0) {
                v.val.u |= AMDEFERRED;
                return (v.val.u);
            }
            v.val.u |= AMIXDEFER;
            if (p2->padrb - p2->adrbuf > 6)
                p2_addrovf(p2);
            *p2->padrb++ = 0;
            *p2->padrb++ = 0;
            *p2->padrb++ = (size_t)p2->xsymbol;
            return (v.val.u);

        case '-':
            p2_readop(p2);
            if (p2->tok.u != '(') {
                p2->savop = p2->tok.u;
                p2->tok.u = '-';
                break;
            }
            p2_readop(p2);
            v = p2_express(p2);
            p2_checkrp(p2);
            p2_checkreg(p2, &v);
            v.val.u |= t | AMAUTODECR;
            return (v.val.u);

        case '$':
            p2_readop(p2);
            v = p2_express(p2);
            if (p2->padrb - p2->adrbuf > 6)
                p2_addrovf(p2);
            *p2->padrb++ = v.val.u;
            *p2->padrb++ = v.type.u;
            *p2->padrb++ = (size_t)p2->xsymbol;
            v.val.u      = t | AMIMMED;
            return (v.val.u);

        case '*':
            if (t != 0)
                p2_aerror(p2, '*');
            t = AMDEFERRED;
            p2_readop(p2);
            continue;

        default:
            break;
        }
        break; // only continue statement loops...
    }

    v = p2_express(p2);
    if (p2->tok.u == '(') {
        p2_readop(p2);
        if (p2->padrb - p2->adrbuf > 6)
            p2_addrovf(p2);
        *p2->padrb++ = v.val.u;
        *p2->padrb++ = v.type.u;
        *p2->padrb++ = (size_t)p2->xsymbol;
        v            = p2_express(p2);
        p2_checkreg(p2, &v);
        p2_checkrp(p2);
        v.val.u |= AMINDEXED | t;
        return (v.val.u);
    }

    if (v.type.u == TYPEREGIS) {
        p2_checkreg(p2, &v);
        v.val.u |= t;
        return (v.val.u);
    }

    v.type.u |= 0100000; // relative address
    v.val.u -= (dot(p2) + 4);
    if (p2->padrb != &p2->adrbuf[0])
        v.val.u -= 2;
    if (p2->padrb - p2->adrbuf > 6)
        p2_addrovf(p2);
    *p2->padrb++ = v.val.u;
    *p2->padrb++ = v.type.u;
    *p2->padrb++ = (size_t)p2->xsymbol;
    return (AMRELATIVE | t);
}

//
// Report address buffer overflow and exit pass2.
// Called when more than two deferred addresses would be needed in adrbuf.
// Inputs: p2 (line).
// Outputs: Message to stdout; p2_aexit(p2, 1).
//
void p2_addrovf(struct pass2 *p2)
{
    printf("addrovf: address over flow, line %d\n", p2->line);
    p2_aexit(p2, 1);
}

//
// Verify value is a valid register (0..7); if not, error and set value to 0/UNDEF.
// Called from p2_opline and p2_address after parsing (reg) or register operand.
// Inputs: p2 (for p2_aerror), v (value to check and possibly clobber).
// Outputs: v->val.i = 0, v->type.i = TYPEUNDEF on error.
//
void p2_checkreg(struct pass2 *p2, struct value *v)
{
    if (v->val.u > 7 || (v->type.u > TYPEABS && v->type.u < TYPEOPFD)) {
        p2_aerror(p2, 'a');
        v->val.i  = 0;
        v->type.i = TYPEUNDEF;
    }
}

//
// Require current token to be ')' and advance.
// Called after parsing expression inside parentheses in p2_address.
// Inputs: p2 (tok).
// Outputs: If ')', p2_readop; else p2_aerror and return.
//
void p2_checkrp(struct pass2 *p2)
{
    if (p2->tok.i != ')') {
        p2_aerror(p2, ')');
        return;
    }
    p2_readop(p2);
    return;
}

//
// Record branch target in brtab; return 0 if short branch fits, 2 if long form needed.
// Called on pass 0 for jbr/jcc to decide instruction size.
// Inputs: p2 (brtab, brtabp, brdelt, dot), v (target address).
// Outputs: Returns 0 if offset in -254..256 (word), 2 if out of range (need jmp); sets brtab bit
// when long.
//
int p2_setbr(struct pass2 *p2, int v)
{
    if (p2->brtabp > BRLEN) // no more room in table...
        return (2);
    ++p2->brtabp;
    if ((v -= dot(p2)) > 0)
        v -= p2->brdelt;
    if (v >= -254 && v <= 256) // in range...
        return (0);
    p2->brtab[(p2->brtabp - 1) >> 3] |= (1 << ((p2->brtabp - 1) & 7)); // out
    return (2);
}

//
// Check whether current brtab entry requires long branch (jmp); advance brtabp.
// Called on pass 1 for jbr/jcc to decide short vs long form.
// Inputs: p2 (brtab, brtabp).
// Outputs: Returns 0 if short branch ok, 1 if long; increments brtabp.
//
int p2_getbr(struct pass2 *p2)
{
    int t;

    if (p2->brtabp > BRLEN)
        return (1);
    t = p2->brtabp++;
    return ((p2->brtab[t >> 3] >> (t & 7)) & 1);
}

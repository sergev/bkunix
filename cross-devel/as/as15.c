//
// as - PDP/11 assembler, Part I
//
// Main scanner routine and some subroutines
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include "as.h"
#include "as1.h"

//
// Read next token from input and set p1->tok (and numval where needed); emit token to pass1 output.
// Called from assem and opline/express to get operands and punctuation.
// Inputs: p1 (chartab, savop, eos_flag, schar, esctab); input via rch.
// Outputs: p1->tok set to next token; token written via aputw; numval set for strings/ints.
//
void readop(struct pass1 *p1)
{
    unsigned char c;
    int i;

    if ((p1->tok.i = p1->savop) != 0) {
        p1->savop = 0;
        return;
    }

    while (1) {
        while ((c = p1->chartab[p1->tok.i = rch(p1)]) == CHARWHITE)
            ;
        if (c != 0 && c < 128)
            goto rdname;

        switch (c) {
        case CHARSTRING: // <...>
            p1->tok.i = '<';
            aputw(p1);
            p1->numval = 0;
            while (c = rsch(p1), !p1->eos_flag) {
                p1->tok.i = c | STRINGFLAG;
                aputw(p1);
                ++p1->numval;
            }
            p1->tok.i = -1;
            aputw(p1);
            p1->tok.i = '<';
            return;

        case CHARLF: // character is the token
        case CHARTOKEN:
            break;

        case CHARSKIP: // / - comment
            while ((c = rch(p1)) != TOKEOF && c != '\n')
                ;
            p1->tok.i = c; // newline or eof
            break;

        case CHARNAME:
        rdname:
            p1->ch = p1->tok.i;
            if (c < '0' || c > '9') {
                rname(p1, c);
                return;
            }
            // fall thru since it is a number

        case CHARNUM:
            if (!number(p1))
                break; // really a temporary label
            p1->numval = p1->num_rtn;
            p1->tok.i  = TOKINT;
            aputw(p1);
            p1->tok.i = p1->numval;
            aputw(p1);
            p1->tok.i = TOKINT;
            return;

        case CHARSQUOTE:
        case CHARDQUOTE:
            if (c == CHARSQUOTE)
                p1->numval = rsch(p1);
            else {
                p1->numval = rsch(p1);
                p1->numval |= rsch(p1) << 8;
            }
            p1->tok.i = TOKINT;
            aputw(p1);
            p1->tok.i = p1->numval;
            aputw(p1);
            p1->tok.i = TOKINT;
            return;

        case CHARGARB:
            aerror(p1, 'g');
            continue;

        case CHARESCP:
            c = rch(p1);
            for (i = 0; p1->esctab[i] != 0; i += 2) {
                if (p1->esctab[i] == c) {
                    p1->tok.i = p1->esctab[i + 1];
                    break;
                }
            }
            break;

        case CHARFIXOR:
            p1->tok.i = TOKVBAR;
            break;
        }

        aputw(p1);
        return;
    }
}

//
// Read one character inside a <...> string; handle escape and set eos_flag on '>'.
// Called from readop when inside CHARSTRING to get string content.
// Inputs: p1 (schar, eos_flag); next char from input.
// Outputs: Returns character value (or escape replacement); eos_flag set if char is '>'.
// TOKEOF or newline triggers aerror and aexit; backslash uses schar table; otherwise eos_flag = (c
// == '>').
//
char rsch(struct pass1 *p1)
{
    char c;
    int i;

    if ((c = rch(p1)) == TOKEOF || c == '\n') {
        aerror(p1, '<');
        aexit(p1);
    }
    p1->eos_flag = 0;
    if (c == '\\') {
        c = rch(p1);
        for (i = 0; p1->schar[i] != 0; i += 2) {
            if (p1->schar[i] == c)
                return (p1->schar[i + 1]);
        }
        return (c);
    }
    p1->eos_flag = (c == '>');
    return (c);
}

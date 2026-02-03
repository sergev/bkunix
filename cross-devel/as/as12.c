//
// as - PDP/11 assember, Part 1
//
// Some support routines
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as1.h"

//
// Print an assembler error message keyed by character code and set errflg.
// Called from parser/expression code when syntax or semantic errors are found.
// Inputs: p1 (curarg, line for location; errflg incremented), c (error code).
// Outputs: Prints "file:line: message" to stdout; increments p1->errflg.
// Switch maps c to message string; unknown c prints "Error 'c'".
//
void aerror(struct pass1 *p1, int c)
{
    char *msg = 0;

    switch (c) {
    case 'x':
        msg = "Syntax error";
        break;
    case '.':
        msg = "Dot '.' expected";
        break;
    case 'm':
        msg = "Invalid label";
        break;
    case 'f':
        msg = "Invalid temporary label";
        break;
    case 'i':
        msg = "Unterminated .endif";
        break;
    case 'g':
        msg = "Unexpected character";
        break;
    case '<':
        msg = "Unterminated <> string";
        break;
    case 'a':
        msg = "Invalid register name";
        break;
    case 'U':
        msg = "Undefined identifier in .if";
        break;
    case '~':
        msg = "Internal error";
        break;
    case '*':
        msg = "Error at '*'";
        break;
    case ')':
        msg = "Expected ')'";
        break;
    case 'e':
        msg = "Invalid expression";
        break;
    case ']':
        msg = "Expected ']'";
        break;
    }
    printf("%s:%d: ", *p1->curarg, p1->line);
    if (msg)
        printf("%s\n", msg);
    else
        printf("Error '%c'\n", c);
    ++p1->errflg;
}

//
// Write current token (2 bytes) to pass1 output file, unless inside .if and not newline.
// Called by scanner/symbol code to record token stream for pass2.
// Inputs: p1 (pof, ifflg, tok).
// Outputs: Writes 2-byte token to p1->pof when not suppressed by .if.
// Suppresses non-newline tokens inside .if to keep line count; writes little-endian tok.i.
//
void aputw(struct pass1 *p1)
{
    char buf[2];

    if (!p1->ifflg || p1->tok.i == '\n') {
        buf[0] = (unsigned char)p1->tok.i;
        buf[1] = (unsigned char)(p1->tok.i >> 8);
        if (write(p1->pof, buf, 2) != 2)
            fprintf(stderr, "aputw: write error\n");
    }
}

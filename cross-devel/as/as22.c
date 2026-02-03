/*
 * AS - PDP/11 Assembler, Part II
 *
 * Miscellaneous support routines
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as2.h"

/*
        Routine to output a word to output, with relocation
*/
void p2_outw(struct pass2 *p2, int type, int val)
{
    unsigned t;

    if (DEBUG)
        printf("outw type %o val %o ", type, val);
    if (dotrel(p2) == TYPEBSS) {
        p2_aerror(p2, 'x');
        return;
    }
    if (dot(p2) & 1) {
        p2_aerror(p2, 'o');
        p2_outb(p2, TYPEUNDEF, val);
        return;
    }
    p2->symtab[0].val.u += 2;
    if (p2->passno == 0)
        return;
    t = ((type & ENDTABFLAG) >> 15) & 1;
    type &= ~ENDTABFLAG;
    if (type == TYPEEXT) {
        p2->outmod = 0666;
        type       = (((struct value *)p2->xsymbol - p2->usymtab) << 3) | 4;
    } else {
        if ((type &= ~TYPEEXT) >= TYPEOPFD) {
            if (type == TYPEOPEST || type == TYPEOPESD)
                p2_aerror(p2, 'r');
            type = TYPEABS;
        }
        if (type >= TYPETXT && type <= TYPEBSS) {
            if (t == 0)
                val += dotdot(p2);
        } else {
            if (t != 0)
                val -= dotdot(p2);
        }
        if (--type < 0)
            type = TYPEUNDEF;
    }

    type = (type << 1) | t;
    p2_aputw(p2, &p2->txtp, val);
    *p2->tseekp += 2;
    p2_aputw(p2, &p2->relp, type);
    *p2->rseekp += 2;
    return;
}

/*
        Routine to output a byte value
*/
void p2_outb(struct pass2 *p2, int type, int val)
{
    if (dotrel(p2) == TYPEBSS) {
        p2_aerror(p2, 'x');
        return;
    }
    if (type > TYPEABS)
        p2_aerror(p2, 'r');
    if (p2->passno != 0) {
        if (!(dot(p2) & 1)) {
            p2_aputw(p2, &p2->txtp, val);
            p2_aputw(p2, &p2->relp, 0);
            *p2->rseekp += 2;
            *p2->tseekp += 2;
        } else {
            *((char *)p2->txtp.slot - 1) = val;
        }
    }
    p2->symtab[0].val.u++;
}

/*
        Display file, line and error code for errors
*/
void p2_aerror(struct pass2 *p2, int c)
{
    char *msg = 0;

    switch (c) {
    case 'u':
        msg = "Unknown symbol";
        break;
    case 'x':
        msg = "Data in .bss section";
        break;
    case 'o':
        msg = "Odd address";
        break;
    case 'r':
        msg = "Relocatable value not allowed";
        break;
    case '.':
        msg = "Dot-relative expression required";
        break;
    case 'b':
        msg = "Branch offset too big";
        break;
    case 'a':
        msg = "Incorrect operand value";
        break;
    case ')':
        msg = "Requred ')'";
        break;
    case ']':
        msg = "Requred ']'";
        break;
    case 'e':
        msg = "Bad expression";
        break;
    }
    printf("%s:%d: ", p2->argb, p2->line);
    if (msg)
        printf("%s\n", msg);
    else
        printf("Error '%c'\n", c);

    p2->outmod = 0666; /* not executable */
}

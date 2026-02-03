//
// as - PDP/11 assembler part 1 global data and init (now in struct pass1)
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include "as.h"
#include "as1.h"

//
// Scanner character table, values match preprocessor vars CHAR*
//
static const char chartab_init[] = {
    -014, -014, -014, -014, -002, -014, -014, -014, //
    -014, -022, -02,  -014, -014, -022, -014, -014, //
    -014, -014, -014, -014, -014, -014, -014, -014, //
    -014, -014, -014, -014, -014, -014, -014, -014, //
    -022, -020, -016, -006, -020, -020, -020, -012, //
    -020, -020, -020, -020, -020, -020, 0056, -006, //
    0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067, //
    0070, 0071, -020, -002, -000, -020, -014, -014, //
    -014, 0101, 0102, 0103, 0104, 0105, 0106, 0107, //
    0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117, //
    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127, //
    0130, 0131, 0132, -020, -024, -020, -020, 0137, //
    -014, 0141, 0142, 0143, 0144, 0145, 0146, 0147, //
    0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157, //
    0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167, //
    0170, 0171, 0172, -014, -026, -014, 0176, -014, //
};

static const char schar_init[] = { 'n', 012, 't', 011, 'e', TOKEOF, '0', 0,
                                   'r', 015, 'a', 006, 'p', 033,    0,   -1 };

static const char esctab_init[] = { '/', '/', '<', TOKLSH, '>', TOKRSH, '%', TOKVBAR, 0, 0 };

//
// Initialize pass1 state: clear hash table, set symtab bounds, zero savdot/curfb/ifflg/etc., copy
// tables. Called once from asm_pass1 before processing. Inputs: p1 (struct pass1 to fill). Outputs:
// p1 fully initialized for first pass (hshtab, usymtab, symend, chartab, schar, esctab).
//
void pass1_init(struct pass1 *p1)
{
    int i;

    for (i = 0; i < HSHSIZ; i++)
        p1->hshtab[i] = 0;
    p1->usymtab   = p1->symtab + SYMBOLS;
    p1->symend    = p1->symtab + SYMBOLS;
    p1->savdot[0] = p1->savdot[1] = p1->savdot[2] = 0;
    for (i = 0; i < 10; i++) {
        p1->curfbr[i] = 0;
        p1->curfb[i]  = -1;
    }
    p1->nxtfb.label = 0;
    p1->nxtfb.val   = 0;
    p1->errflg      = 0;
    p1->ifflg       = 0;
    p1->globflag    = 0;
    p1->eos_flag    = 0;
    p1->line        = 0;
    p1->savop       = 0;
    p1->ch          = 0;
    p1->numval      = 0;
    for (i = 0; i < (int)(sizeof chartab_init / sizeof chartab_init[0]) && i < PASS1_CHARTAB_SIZE;
         i++)
        p1->chartab[i] = chartab_init[i];
    for (i = 0; i < (int)(sizeof schar_init / sizeof schar_init[0]) && i < PASS1_SCHAR_SIZE; i++)
        p1->schar[i] = schar_init[i];
    for (i = 0; i < (int)(sizeof esctab_init / sizeof esctab_init[0]) && i < PASS1_ESCTAB_SIZE; i++)
        p1->esctab[i] = esctab_init[i];
}

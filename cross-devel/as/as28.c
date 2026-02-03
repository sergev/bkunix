/*
 * AS - PDP/11 Assembler part 2
 *
 * Global data and init (now in struct pass2)
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

#include "as.h"
#include "as2.h"

/*
        Relocation combination tables for various operators
*/
static int reltp2[] = {
    0, 0,   0,  0,  0,  0,   //
    0, -1,  2,  3,  4,  040, //
    0, 2,   -2, -2, -2, -2,  //
    0, 3,   -2, -2, -2, -2,  //
    0, 4,   -2, -2, -2, -2,  //
    0, 040, -2, -2, -2, -2,  //
};

static int reltm2[] = {
    0, 0,  0,  0,  0,  0,   //
    0, -1, 2,  3,  4,  040, //
    0, -2, 1,  -2, -2, -2,  //
    0, -2, -2, 1,  -2, -2,  //
    0, -2, -2, -2, 1,  -2,  //
    0, -2, -2, -2, -2, -2,  //
};

static int relte2[] = {
    0, 0,  0,  0,  0,  0,  //
    0, -1, -2, -2, -2, -2, //
    0, -2, -2, -2, -2, -2, //
    0, -2, -2, -2, -2, -2, //
    0, -2, -2, -2, -2, -2, //
    0, -2, -2, -2, -2, -2, //
};

void pass2_init(struct pass2 *p2)
{
    int i;

    p2->hdr.txtmagic   = 0407;
    p2->hdr.atxtsiz[0] = p2->hdr.atxtsiz[1] = p2->hdr.atxtsiz[2] = 0;
    p2->hdr.symsiz = p2->hdr.stksiz = p2->hdr.exorig = p2->hdr.unused = 0;

    p2->tseekp = &p2->aseek[0];
    p2->rseekp = &p2->relseek[0];
    p2->nxtfb  = &p2->curfb[10];

    p2->brtabp    = 0;
    p2->brdelt    = 0;
    p2->savdot[0] = p2->savdot[1] = p2->savdot[2] = 0;
    p2->defund                                    = 0;
    p2->passno                                    = 0;
    p2->outmod                                    = 0777;
    p2->line                                      = 0;
    p2->savop                                     = 0;
    p2->xsymbol                                   = 0;
    p2->swapf                                     = 0;
    p2->rlimit                                    = 0;
    p2->datbase                                   = 0;
    p2->bssbase                                   = 0;
    p2->numval                                    = 0;
    p2->maxtyp                                    = 0;

    for (i = 0; i < (int)(sizeof p2->brtab / sizeof p2->brtab[0]); i++)
        p2->brtab[i] = 0;
}

int *pass2_reltp2(struct pass2 *p2)
{
    (void)p2;
    return reltp2;
}
int *pass2_reltm2(struct pass2 *p2)
{
    (void)p2;
    return reltm2;
}
int *pass2_relte2(struct pass2 *p2)
{
    (void)p2;
    return relte2;
}

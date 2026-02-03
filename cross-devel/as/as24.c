//
// AS - PDP/11 Assembler, Part II
//
// More miscellaneous support routines
//
// This file is part of BKUNIX project, which is distributed
// under the terms of the GNU General Public License (GPL).
// See the accompanying file "COPYING" for more details.
//
#include <stdio.h>

#include "as.h"
#include "as2.h"

//
// Initialize an output buffer at a given seek offset; slot at offset within 512-byte buffer.
// Called to set txtp/relp before writing header or at symseek for symbol table.
// Inputs: p2 (unused), p (out_buf), o (seek offset).
// Outputs: p->slot, p->max, p->seek set (slot = buf + (o & 0777)).
// Buffer is 512 bytes; slot placed at offset mod 512 for direct indexing.
//
void p2_oset(struct pass2 *p2, struct out_buf *p, int o)
{
    (void)p2;
    p->slot = p->buf + (o & 0777);
    p->max  = p->buf + sizeof p->buf;
    p->seek = o;
    if (DEBUG)
        printf("\noset: offset %x slot %d seek %d\n", o, (int)(p->slot - p->buf) / 2, p->seek);
}

//
// Append a 16-bit word to a buffered output; flush buffer if full.
// Used for text and relocation output; little-endian write.
// Inputs: p2 (for p2_flush), p (out_buf), v (word value).
// Outputs: v written to p->slot (2 bytes); slot advanced; buffer flushed if at max.
//
void p2_aputw(struct pass2 *p2, struct out_buf *p, int v)
{
    char *pi;

    pi = p->slot;
    if (pi < p->max) {
        *pi++   = v;
        *pi++   = v >> 8;
        p->slot = pi;
    } else {
        p2_flush(p2, p);
        *(p->slot++) = v;
        *(p->slot++) = v >> 8;
    }
    if (DEBUG)
        printf("aputw  %s %o slot %d ", (p == &p2->relp) ? "rel" : "txt", v,
               (int)(p->slot - p->buf) / 2);
    if (debug_flag)
        printf("--- write outfile (%s): 0x%04x (%o)\n", (p == &p2->relp) ? "rel" : "txt",
               (unsigned)v, (unsigned)v);
}

//
// Write buffered data to fout at current seek and advance seek to next block.
// Called before appending symbol table or at end of pass2.
// Inputs: p2 (fout), p (out_buf with slot and seek).
// Outputs: Bytes from (seek&0777) to slot written at seek; seek rounded up to next 512.
//
void p2_flush(struct pass2 *p2, struct out_buf *p)
{
    char *addr;
    int bytes;

    addr  = p->buf + (p->seek & 0777);
    bytes = p->slot - addr;
    if (bytes == 0)
        return;

    if (debug_flag)
        printf("--- write outfile (flush): %d bytes at offset 0x%x\n", bytes, p->seek);
    lseek(p2->fout, (long)p->seek, 0);
    write(p2->fout, addr, bytes);

    p->seek = (p->seek | 0777) + 1;
    p->slot = p->buf;
}

//
// Read next token from pass1 token file into p2->tok; resolve symbol pointers.
// Called from p2_assem and p2_opline to get operands.
// Inputs: p2 (savop, fin); token file position.
// Outputs: p2->tok set; savop consumed if set; tok.v set for symbol tokens.
//
void p2_readop(struct pass2 *p2)
{
    p2->tok = p2->savop;
    if (p2->tok.i != 0) {
        p2->savop.i = 0;
        return;
    }
    p2_agetw(p2);
    if (p2->tok.u > TOKSYMBOL) {
        if (p2->tok.u >= USYMFLAG) {
            p2->tok.u -= USYMFLAG;
            p2->tok.v = &global_symtab[SYMBOLS + p2->tok.u].v;
        } else {
            p2->tok.u -= PSYMFLAG;
            p2->tok.v = &global_symtab[p2->tok.u].v;
        }
    }
}

//
// Read one 16-bit word from token file into p2->tok.u; return FALSE on EOF.
// Called by p2_readop and asm_pass2 when reading token stream or symbol/fb data.
// Inputs: p2 (savop, fin).
// Outputs: tok.u = word (or TOKEOF); savop cleared if was set; returns TRUE if word read.
//
int p2_agetw(struct pass2 *p2)
{
    unsigned char buf[2];

    p2->tok = p2->savop;
    if (p2->tok.u != 0) {
        p2->savop.u = 0;
        return (TRUE);
    }
    if (read(p2->fin, buf, 2) < 2) {
        p2->tok.u = TOKEOF;
        return (FALSE);
    }
    p2->tok.u = buf[0] | buf[1] << 8;
    if (debug_flag) {
        const char *name = (p2->fin == p2->fbfil)  ? "atmp2"
                           : (p2->fin == p2->txtfil) ? "atmp1"
                                                      : "?";
        printf("--- read %s: 0x%04x (%o)\n", name, (unsigned)p2->tok.u, (unsigned)p2->tok.u);
    }
    return (TRUE);
}

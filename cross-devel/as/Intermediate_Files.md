# Intermediate Files

The PDP-11 assembler is a two-pass program: pass 1 parses the source and writes two temporary files; pass 2 reads those files and produces the final object (a.out). The symbol table (opcodes and user symbols) is a single global array (`global_symtab`, `global_symend` in [as1.h](as1.h)) used by both passes; it is not written to a file. Both temp files are created with `mkstemp()` and are removed on exit (normal or error); they are not intended to persist.

| File  | Pass 1 handle   | Pass 2 handle          | Purpose                                 |
| ----- | --------------- | ---------------------- | --------------------------------------- |
| atmp1 | `p1.pof`        | `p2.txtfil` → `p2.fin` | Token stream (replay of source)          |
| atmp2 | `p1.fbfil`      | `p2.fbfil` → `p2.fin`  | Forward branch table (temporary labels) |

Pass 2 reads the files in this order: atmp2 (forward branches), then atmp1 (token stream) for assembly. See `asm_pass2()` in [as21.c](as21.c).

---

## 1. atmp1 — Token stream

### Purpose

Stores the stream of tokens produced in pass 1 so pass 2 can re-play the same program without re-parsing the source. Pass 2 uses this stream as `p2.fin` during `p2_assem()`.

### Usage

- **Creation (pass 1):** In [as11.c](as11.c), `p1.pof = f_create(&p1, atmp1)` creates the file from the template `/tmp/atm1XXXXXX`. Every token emitted by the lexer/parser is written via `aputw()` in [as12.c](as12.c) (called from [as14.c](as14.c), [as15.c](as15.c), etc.).
- **Read (pass 2):** In [as21.c](as21.c), `p2.txtfil = p2_ofile(&p2, p2.atmp1)` opens the file; then `p2.fin = p2.txtfil` before `p2_assem()`. Words are read by `p2_agetw()` in [as24.c](as24.c), which is used by `p2_readop()` and thus by the whole of pass 2 assembly.
- **Deletion:** `unlink(p2->atmp1)` in [as21.c](as21.c) `p2_aexit()`; on pass 1 error, `unlink(atmp1)` in [as11.c](as11.c) `aexit()`.

### Internal data layout

Unstructured byte stream. Each **token** is one **16-bit word**, little-endian (low byte first).

- Written in [as12.c](as12.c) `aputw()`: `buf[0] = tok.i`, `buf[1] = tok.i >> 8`; `write(p1->pof, buf, 2)`. Tokens are only written when not inside a skipped `.if` block (or when the token is newline).
- Token values include: small integers (e.g. `TOKINT` 01, `TOKEOF` 04), character codes, `TOKSYMBOL` (0200) + index, `PSYMFLAG` (01000) + permanent symbol index, `USYMFLAG` (04000) + user symbol index. See [as.h](as.h) for these constants.

There are no record boundaries; pass 2 must interpret the stream in the same order as pass 1 produced it.

---

## 2. atmp2 — Forward branch table

### Purpose

Records each temporary label definition (`n:` with n in 0–9) so pass 2 can resolve forward references. One entry is written for each such label seen in pass 1.

### Usage

- **Creation (pass 1):** In [as11.c](as11.c), `p1.fbfil = f_create(&p1, atmp2)` creates the file from `/tmp/atm2XXXXXX`. For each `n:` in the source, [as13.c](as13.c) `write_fb()` appends one 4-byte record.
- **Read (pass 2):** In [as21.c](as21.c), `p2.fin = p2.fbfil`; a loop reads two 16-bit words per entry via `p2_agetw()`, fills `p2.fbtab`, and sets `p2.endtable`. Reading stops when `p2_agetw()` returns false (EOF).
- **Deletion:** `unlink(p2->atmp2)` in [as21.c](as21.c) `p2_aexit()`; on pass 1 error, `unlink(atmp2)` in [as11.c](as11.c) `aexit()`.

### Internal data layout

Sequence of **4-byte records**. Each record corresponds to `struct fb_tab` in [as.h](as.h).

| Bytes | Content   | Description |
| ----- | --------- | ----------- |
| 0–1   | `label`   | 16-bit, little-endian. High byte = temporary label number (0–9); low byte = `dotrel` (segment type). Set in [as13.c](as13.c) as `nxtfb.label = (i<<8) | dotrel`. |
| 2–3   | `val`     | 16-bit, little-endian. Value of `.` (location counter) at the `n:` definition. Written as `nxtfb.val = dot` in [as13.c](as13.c). |

Written in [as13.c](as13.c) `write_fb()`: `buf[0] = b->label`, `buf[1] = b->label >> 8`, `buf[2] = b->val`, `buf[3] = b->val >> 8`; `write(f, buf, 4)`.

---

## 3. Symbol table (in memory)

### Purpose

Opcodes and user-defined symbols live in a single global table (`global_symtab[SYMBOLS + USERSYMBOLS]`, `global_symend` in [as1.h](as1.h)). Pass 1 fills opcodes in `setup()` and user symbols via `add_symbol()`; pass 2 uses the same table for relocation and for appending the user symbol records to a.out. No temporary file is used.

### Usage

- **Pass 1:** In [as11.c](as11.c) `setup()`, `global_symtab[0 .. opcode_table_size-1]` is filled from `opcode_table`. In [as18.c](as18.c) `pass1_init()`, `global_symend = global_symtab + SYMBOLS`. New user symbols are added in [as14.c](as14.c) via `add_symbol()` (writes into `global_symtab[SYMBOLS]` onward, advances `global_symend`).
- **Pass 2:** In [as21.c](as21.c), `p2.hdr.symsiz` is set from `global_symend - (global_symtab + SYMBOLS)`; relocation runs over user symbols; the a.out symbol table is written by iterating `global_symtab + SYMBOLS` to `global_symend` (8-byte name and type/val per symbol).

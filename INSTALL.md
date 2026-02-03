# Building BKUNIX

This document explains how to build **BKUNIX** from the source tree: installing
the cross-development toolchain, generating a bootable disk image, and running
it under a BK emulator.

## 1) Install the cross-development toolchain

From the repository root, run:

```sh
make -C cross-devel install
make -C src/libc install
make -C fsutil install
```

By default, the following utilities are installed into `/usr/local/bin`:

- `pdp11-cc` — front end for Ritchie’s C compiler
- `pdp11-pcc` — front end for Johnson’s Portable C Compiler (PCC)
- `pdp11-asm` — AT&T assembler
- `pdp11-ld` — linker
- `pdp11-ar` — archiver
- `pdp11-disasm` — disassembler
- `pdp11-nm` — object name list utility
- `pdp11-size` — object size utility
- `pdp11-strip` — object strip utility
- `u6-fsutil` — Unix V6 filesystem utility

Some supporting files are installed into `/usr/local/lib/pdp11`:

- `c0` — first pass of Ritchie’s C compiler
- `c1` — second pass of Ritchie’s C compiler
- `ccom` — Johnson’s portable C compiler
- `cpp` — C preprocessor
- `c2` — optimizer for assembler output of C compilers
- `libcrt.a` — runtime library for `long` and `unsigned` arithmetic
- `crt0.o` — C runtime startup routine
- `libc.a` — standard C library

### Installing somewhere other than `/usr/local`

If you prefer a different prefix, use `DESTDIR=...`:

```sh
make -C cross-devel install DESTDIR=~/.local
make -C src/libc install DESTDIR=~/.local
make -C fsutil install DESTDIR=~/.local
```

## 2) Build BKUNIX disk images

Enter the `src/` directory and build:

```sh
make
```

This compiles the kernel and userland utilities and produces a bootable disk
image:

- `root.bkd` — bootable root filesystem image

To inspect the disk image contents:

```sh
u6-fsutil -v root.bkd
```

## 3) Run BKUNIX under an emulator

Install a BK emulator and ensure it is available in your `PATH`. Then, from
within `src/`, run:

```sh
make run
```

At the `boot:` prompt, type `bkunix`. You should then see a root shell prompt
`#`.

To exit the simulator, close the BK emulator window.

### BK emulator source

You can download the BK emulator sources from GitHub:
[emestee/bk-emulator](https://github.com/emestee/bk-emulator).

## More information

- Project home: <https://github.com/sergev/bkunix>

Welcome to the world of ancient Unix.

---

Best wishes,
BKUNIX project team,
Leonid Broukhis, Serge Vakulenko

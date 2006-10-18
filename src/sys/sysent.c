/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include "param.h"
#include "systm.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
struct sysent sysent[64] = {
	0, nullsys,			/*  0 = indir */
	0, rexit,			/*  1 = exit */
	0, sfork,			/*  2 = fork */
	2, read,			/*  3 = read */
	2, write,			/*  4 = write */
	2, open,			/*  5 = open */
	0, close,			/*  6 = close */
	0, wait,			/*  7 = wait */
	2, creat,			/*  8 = creat */
	2, link,			/*  9 = link */
	1, unlink,			/* 10 = unlink */
	2, exec,			/* 11 = exec */
	1, chdir,			/* 12 = chdir */
	0, gtime,			/* 13 = time */
	3, mknod,			/* 14 = mknod */
	2, chmod,			/* 15 = chmod */
	2, nullsys,			/* 16 = chown */
	1, sbreak,			/* 17 = break */
	2, stat,			/* 18 = stat */
	2, seek,			/* 19 = seek */
	0, getpid,			/* 20 = getpid */
#ifdef MNTOPTION
	3, smount,			/* 21 = mount */
	1, sumount,			/* 22 = umount */
#else
	0, nosys,			/* 21 = mount */
	0, nosys,			/* 22 = umount */
#endif
	0, nullsys,			/* 23 = setuid */
	0, getuid,			/* 24 = getuid */
	0, stime,			/* 25 = stime */
	3, nullsys,			/* 26 = ptrace */
	0, alarm,			/* 27 = alarm */
	1, fstat,			/* 28 = fstat */
	0, pause,			/* 29 = pause */
	1, nullsys,			/* 30 = smdate; inoperative */
	1, stty,			/* 31 = stty */
	1, gtty,			/* 32 = gtty */
	0, nosys,			/* 33 = x */
	0, nullsys,			/* 34 = nice */
	0, nosys,			/* 35 = sleep */
	0, sync,			/* 36 = sync */
#ifdef BGOPTION
	0, kill,			/* 37 = kill */
#endif
#ifndef BGOPTION
	1, nosys,			/* 37 = kill */
#endif
	0, nosys,			/* 38 = switch */
	0, nosys,			/* 39 = x */
	0, nosys,			/* 40 = x */
	0, dup,				/* 41 = dup */
	0, nosys,			/* 42 = pipe */
	1, nullsys,			/* 43 = times */
	4, nosys,			/* 44 = prof */
	0, nosys,			/* 45 = tiu */
	0, nullsys,			/* 46 = setgid */
	0, nullsys,			/* 47 = getgid */
	2, ssig,			/* 48 = sig */
	0, nosys,			/* 49 = x */
	0, nosys,			/* 50 = x */
	0, nosys,			/* 51 = x */
	0, nosys,			/* 52 = x */
	0, nosys,			/* 53 = x */
	0, nosys,			/* 54 = x */
	0, nosys,			/* 55 = x */
	0, nosys,			/* 56 = x */
	0, nosys,			/* 57 = x */
	0, nosys,			/* 58 = x */
	0, nosys,			/* 59 = x */
	0, nosys,			/* 60 = x */
	0, nosys,			/* 61 = x */
	0, nosys,			/* 62 = x */
	0, dup2,			/* 63 = dup2 (as in Linux) */
};

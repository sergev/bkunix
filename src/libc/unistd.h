#include <ansidecl.h>

/*
 * Terminate the calling process.
 */
void exit PARAMS((int));

/*
 * Create a new process.
 */
int fork PARAMS((void));

/*
 * Read/write file.
 */
int read PARAMS((int, char*, int));
int write PARAMS((int, char*, int));

/*
 * Open or create a file for reading or writing.
 */
int open PARAMS((char*, int));

/*
 * Close a file.
 */
int close PARAMS((int));

/*
 * Get a status of child process.
 */
int wait PARAMS((int*));

/*
 * Create a new file.
 */
int creat PARAMS((char*, int));

/*
 * Make a file link.
 */
int link PARAMS((char*, char*));

/*
 * Remove directory entry.
 */
int unlink PARAMS((char*));

/*
 * Execute a file.
 */
int execv PARAMS((char*, char**));

/*
 * Change current working directory.
 */
int chdir PARAMS((char*));

/*
 * Get time of day.
 */
void time PARAMS((long*));

/*
 * Make a special file node.
 */
int mknod PARAMS((char*, int, int));

/*
 * Change mode of file.
 */
int chmod PARAMS((char*, int));

/*
 * Increase data segment size.
 */
char *sbrk PARAMS((int));

/*
 * Get file status by file name.
 */
int stat PARAMS((char*, struct stat*));

/*
 * Reposition read/write file offset.
 */
int seek PARAMS((int, unsigned int, int));

/*
 * Get calling process identification.
 */
int getpid PARAMS((void));

/*
 * Get user identification.
 */
int getuid PARAMS((void));

/*
 * Set time of day.
 */
int stime PARAMS((long*));

/*
 * Get/set value of process alarm timer.
 */
int alarm PARAMS((int));

/*
 * Get file status by file descriptor.
 */
int fstat PARAMS((int, struct stat*));

/*
 * Stop until signal.
 */
void pause PARAMS((void));

/*
 * Set and get terminal state.
 */
int stty PARAMS((int, int*));
int gtty PARAMS((int, int*));

/*
 * Force a write of modified buffers out to disk.
 */
void sync PARAMS((void));

/*
 * Duplicate an existing file descriptor.
 */
int dup PARAMS((int));

/*
 * Set signal handler.
 */
int signal PARAMS((int, int));

#if 0
/*
 * Empty syscalls:
 */
	0, nullsys,			/*  0 = indir */
	2, nullsys,			/* 16 = chown */
	0, nullsys,			/* 23 = setuid */
	3, nullsys,			/* 26 = ptrace */
	1, nullsys,			/* 30 = smdate; inoperative */
	0, nullsys,			/* 34 = nice */
	1, nullsys,			/* 43 = times */
	0, nullsys,			/* 46 = setgid */
	0, nullsys,			/* 47 = getgid */

/*
 * Unavalable syscalls:
 */
	3, nosys,			/* 21 = mount */
	1, nosys,			/* 22 = umount */
	0, nosys,			/* 33 = x */
	0, nosys,			/* 35 = sleep */
	1, nosys,			/* 37 = kill */
	0, nosys,			/* 38 = switch */
	0, nosys,			/* 39 = x */
	0, nosys,			/* 40 = x */
	0, nosys,			/* 42 = pipe */
	4, nosys,			/* 44 = prof */
	0, nosys,			/* 45 = tiu */
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
	0, nosys,			/* 63 = x */
#endif

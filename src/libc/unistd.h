/*
 * Mini-unix system calls.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _UNISTD_H_
#define _UNISTD_H_ 1

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
 * Get a status of child process -- use <sys/wait.h>.
 * int wait PARAMS((int*));
 */

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
int execl PARAMS((char*, char* /*, ...*/));

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
 * Get file status by file name --  use <sys/stat.h>.
 */
struct stat;
int stat PARAMS((char*, struct stat*));

/*
 * Reposition read/write file offset.
 */
int lseek PARAMS((int, long, int));

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
 * Set and get terminal state -- use <sgtty.h>.
 * int stty PARAMS((int, int*));
 * int gtty PARAMS((int, int*));
 */

/*
 * Force a write of modified buffers out to disk.
 */
void sync PARAMS((void));

/*
 * Duplicate an existing file descriptor.
 */
int dup PARAMS((int));

/*
 * Set signal handler -- use <signal.h>.
 * int signal PARAMS((int, int));
 */

/*
 * Library functions:
 */
int isatty PARAMS((int));

#endif /* _UNISTD_H_ */

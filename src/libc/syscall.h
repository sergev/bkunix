/*
 * System call numbers.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _SYSCALL_H_
#define _SYSCALL_H_ 1

#define SYS_exit	1	/* Terminate the calling process */
#define SYS_fork	2	/* Create a new process */
#define SYS_read	3	/* Read/write file */
#define SYS_write	4
#define SYS_open	5	/* Open or create a file for reading or writing */
#define SYS_close	6	/* Close a file */
#define SYS_wait	7	/* Get a status of child process */
#define SYS_creat	8	/* Create a new file */
#define SYS_link	9	/* Make a file link */
#define SYS_unlink	10	/* Remove directory entry */
#define SYS_exec	11	/* Execute a file */
#define SYS_chdir	12	/* Change current working directory */
#define SYS_time	13	/* Get time of day */
#define SYS_mknod	14	/* Make a special file node */
#define SYS_chmod	15	/* Change mode of file */
#define SYS_break	17	/* Increase data segment size */
#define SYS_stat	18	/* Get file status by file name */
#define SYS_seek	19	/* Reposition read/write file offset */
#define SYS_getpid	20	/* Get calling process identification */
#define SYS_mount	21	/* Mount file systems */
#define SYS_umount	22	/* Unmount file systems */
#define SYS_getuid	24	/* Get user identification */
#define SYS_stime	25	/* Set time of day */
#define SYS_alarm	27	/* Get/set value of process alarm timer */
#define SYS_fstat	28	/* Get file status by file descriptor */
#define SYS_pause	29	/* Stop until signal */
#define SYS_stty	31	/* Set and get terminal state */
#define SYS_gtty	32
#define SYS_sync	36	/* Force a write of modified buffers out to disk */
#define SYS_dup		41	/* Duplicate an existing file descriptor */
#define SYS_signal	48	/* Set signal handler */

#endif /* _SYSCALL_H_ */

/*
 * Terminate the calling process.
 */
void exit(int);					/* 1 = exit */

/*
 * Create a new process.
 */
int fork(void);					/* 2 = fork */

/*
 * Read/write file.
 */
int read(int, char*, int);			/* 3 = read */
int write(int, char*, int);			/* 4 = write */

/*
 * Open or create a file for reading or writing.
 */
int open(char*, int);				/* 5 = open */

/*
 * Close a file.
 */
int close(int);					/* 6 = close */

/*
 * Get a status of child process.
 */
int wait(int*);					/* 7 = wait */

/*
 * Create a new file.
 */
int creat(char*, int);				/* 8 = creat */

/*
 * Make a file link.
 */
int link(char*, char*);				/* 9 = link */

/*
 * Remove directory entry.
 */
int unlink(char*);				/* 10 = unlink */

/*
 * Execute a file.
 */
int execv(char*, char**);			/* 11 = exec */

/*
 * Change current working directory.
 */
int chdir(char*);				/* 12 = chdir */

/*
 * Get time of day.
 */
void time(int*);				/* 13 = time */

/*
 * Make a special file node.
 */
int mknod(char*, int, int);			/* 14 = mknod */

/*
 * Change mode of file.
 */
int chmod(char*, int);				/* 15 = chmod */

/*
 * Increase data segment size.
 */
char *sbrk(int);				/* 17 = break */

/*
 * Get file status by file name.
 */
int stat(char*, int*);				/* 18 = stat */

/*
 * Reposition read/write file offset.
 */
int seek(int, unsigned int, int);		/* 19 = seek */

/*
 * Get calling process identification.
 */
int getpid(void);				/* 20 = getpid */

/*
 * Get user identification.
 */
int getuid(void);				/* 24 = getuid */

/*
 * Set time of day.
 */
int stime(int*);				/* 25 = stime */

/*
 * Get/set value of process alarm timer.
 */
int alarm(int);					/* 27 = alarm */

/*
 * Get file status by file descriptor.
 */
int fstat(int, int*);				/* 28 = fstat */

/*
 * Stop until signal.
 */
void pause(void);				/* 29 = pause */

/*
 * Set and get terminal state.
 */
int stty(int, int*);				/* 31 = stty */
int gtty(int, int*);				/* 32 = gtty */

/*
 * Force a write of modified buffers out to disk.
 */
void sync(void);				/* 36 = sync */

/*
 * Terminate a background process.
 */
void kill(void);				/* 37 = kill */

/*
 * Duplicate an existing file descriptor.
 */
int dup(int);					/* 41 = dup */

/*
 * Set signal handler.
 */
int signal(int, int);				/* 48 = sig */

/*
 * Become a background process.
 */
void bground(void);				/* 63 = bground */

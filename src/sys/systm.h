/*
 * Random set of variables
 * used by more than one
 * routine.
 */
extern char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
extern struct inode *rootdir;		/* pointer to inode of root directory */
extern int	lbolt;			/* time of day in 60th not in time */
extern int	time[2];		/* time in sec from 1970 */
extern int	tout[2];		/* time of day of next sleep */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	struct buf *m_bufp;	/* pointer to superblock */
	struct inode *m_inodp;	/* pointer to mounted on inode */
};
extern struct mount mount[NMOUNT];
extern int	cpid;		/* current process ID */

char *copyin();
char *copyout();
void *memcpy();
void memzero();

/*
 * structure of the system entry table (sysent.c)
 */
struct sysent {
	int	count;		/* argument count */
	void	(*call)();	/* name of handler */
};
extern struct sysent sysent[64];

/*
 * System calls.
 */
void nullsys();
void nosys();
void rexit();
void sfork();
void read();
void write();
void open();
void close();
void wait();
void creat();
void link();
void unlink();
void exec();
void chdir();
void gtime();
void mknod();
void chmod();
void sbreak();
void stat();
void seek();
void getpid();
void getuid();
void stime();
void alarm();
void fstat();
void pause();
void stty();
void gtty();
void sync();
void kill();
void dup();
void ssig();
void bground();

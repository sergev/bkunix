/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef SYSTM_H
#define SYSTM_H 1

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
extern int	user;
#ifdef BGOPTION
extern int	swflg, swwait;
#endif

void *memcpy();
void *memset();
void memzero();
void sleep();
void wakeup();
void ttread();
void ttwrite();
void cinit();
void binit();
void iinit();
void minit();
void dpadd();
void suword();
void savu();
void retu();
void idle();
void rstps();
void update();
int lshift();
int spl0();
int spl7();
int subyte();
int cpass();
int passc();
int issig();
int dpcmp();
int swap();
int newproc();
int bad_user_address();
extern char stop;
void panichalt();
int ttputc();
void ttputs();

#define panic(x)		panichalt(x)

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
void smount();
void sumount();

#endif /* SYSTM_H */

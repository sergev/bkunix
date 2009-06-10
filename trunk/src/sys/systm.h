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
extern long	time;			/* time in sec from 1970 */

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
extern int	bootdev;
#ifdef BGOPTION
extern int	swflg, swwait;
#endif

#ifdef CLOCKOPT
extern int keypress;
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
void suword();
void savu();
void retu();
void idle();
void splx();
void update();
int spl0();
int spl7();
int subyte();
int cpass();
int passc();
int issig();
void swap();
int newproc();
int bad_user_address();
extern char stop;
void panic();
int ttputc();
void ttputs();

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
void lseek();
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
void dup2();
void ssig();
void bground();
void smount();
void sumount();
void fullscr();
void clkinit();

#endif /* SYSTM_H */

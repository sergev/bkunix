#ifndef PROC_H
#define PROC_H 1

/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc
{
	char	p_stat;
	char	p_sig;		/* signal number sent to this process */
	int	p_wchan;	/* event process is awaiting */
	int	p_clktim;	/* time to alarm clock signal */
	int	p_size;		/* size of swap image in WORDS */
};
extern struct proc proc[];

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */

#ifdef BGOPTION
struct proc *swproc;		/* swapper process */
struct proc *bgproc;		/* background process (if any) */

struct swtab {
	int sw_size;
	int sw_blk;
} swtab[];
#endif

#ifdef KERNEL
void signal();
void psignal();
void psig();
void setrun();
void pexit();
#endif

#endif /* PROC_H */

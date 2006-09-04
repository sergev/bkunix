/*
 * A buffer is on the available list, and is liable
 * to be reassigned to another disk block, if and only
 * if it is not marked BUSY.  When a buffer is busy, the
 * available-list pointers can be used for other purposes.
 * Most drivers use the forward ptr as a link in their I/O
 * active queue.
 * A buffer header contains all the information required
 * to perform I/O.
 * Most of the routines which manipulate these things
 * are in bio.c.
 */
struct buf
{
	int	b_flags;		/* see defines below */
	int	*b_link;		/* io driver link word */
	int	b_wcount;		/* transfer count (usu. words) */
	char	*b_addr;		/* low order core address */
	unsigned int b_blkno;		/* block # on device */
	char	b_dev;			/* device number */
	char	b_error;		/* returned after I/O */
};

extern struct buf buf[NBUF];
extern struct buf *bufp[NBUF];		/* pointers to buffer descriptors */

/*
 * Each block device has a devtab, which contains private state stuff
 * and 2 list heads: the b_forw/b_back list, which is doubly linked
 * and has all the buffers currently associated with that major
 * device; and the d_actf/d_actl list, which is private to the
 * device but in fact is always used for the head and tail
 * of the I/O queue for the device.
 * Various routines in bio.c look at b_forw/b_back
 * (notice they are the same as in the buf structure)
 * but the rest is private to each device driver.
 */
struct devtab
{
	char	d_active;		/* busy flag */
	char	d_errcnt;		/* error count (for recovery) */
	struct	buf *d_actf;		/* head of I/O queue */
	struct 	buf *d_actl;		/* tail of I/O queue */
};

/*
 * These flags are kept in b_flags.
 */
#define	B_WRITE	0	/* non-read pseudo-flag */
#define	B_READ	01	/* read when I/O occurs */
#define	B_DONE	02	/* transaction finished */
#define	B_ERROR	04	/* transaction aborted */
#define	B_BUSY	010	/* not on av_forw/back list */
#define	B_DELWRI 01000	/* don't write till block leaves available list */

#ifdef KERNEL
struct buf *alloc();
struct buf *bread();
struct buf *getblk();
void free();
void brelse();
void bwrite();
void bdwrite();
void bflush();
void iowait();
void fdstrategy();
#endif

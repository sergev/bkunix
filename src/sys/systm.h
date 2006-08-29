/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
int	*rootdir;		/* pointer to inode of root directory */
int	lbolt;			/* time of day in 60th not in time */
int	time[2];		/* time in sec from 1970 */
int	tout[2];		/* time of day of next sleep */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
char	cpid;	/* current process ID */
char	regloc[];		/* locs. of saved user registers (trap.c) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
/*#include <fcntl.h>*/
#include <dirent.h>
#include <sys/fs.h>
#include <sys/stat.h>

#define	MAXDUP		10	/* limit on dup blks (per inode) */
#define	MAXBAD		10	/* limit on bad blks (per inode) */
#define	DUP_LIST_SIZE	100	/* num of dup blocks to remember */
#define LINK_LIST_SIZE	20	/* num zero link cnts to remember */

#define STATE_BITS	2	/* bits per inode state */
#define STATE_MASK	3	/* mask for inode state */
#define STATES_PER_BYTE	4	/* inode states per byte */
#define USTATE		0	/* inode not allocated */
#define FSTATE		1	/* inode is file */
#define DSTATE		2	/* inode is directory */
#define CLEAR		3	/* inode is to be cleared */

#define DATA		1	/* flags for scan_inode() */
#define ADDR		0

#define ALTERD		010	/* values returned by scan functions */
#define KEEPON		004
#define SKIP		002
#define STOP		001

#define outrange(fs,x)	((x) < (fs)->isize + 2 || (x) >= (fs)->fsize)

/* block scan function, called by scan_inode for every file block */
typedef int scanner_t PARAMS((struct inode*, unsigned int, void*));

unsigned char	buf_data [FS_BSIZE];	/* buffer data for scan_directory */
unsigned short	buf_bno;		/* buffer block number */
int		buf_dirty;		/* buffer data modified */

unsigned short	dup_list [DUP_LIST_SIZE]; /* dup block table */
unsigned short	*dup_end;		/* next entry in dup table */
unsigned short	*dup_multi;		/* multiple dups part of table */

unsigned short	bad_link_list [LINK_LIST_SIZE]; /* inos with zero link cnts */
unsigned short	*end_bad_link_list;		/* next entry in table */

char		*block_map;		/* primary blk allocation map */
char		*free_map;		/* secondary blk allocation map */
char		*state_map;		/* inode state table */
short		*link_count;		/* link count table */

char		pathname [256];		/* file path name for pass2 */
char		*pathp;			/* pointer to pathname position */
char		*thisname;		/* ptr to current pathname component */

char		*lost_found_name = "lost+found";
unsigned short	lost_found_inode;	/* lost & found directory */

int		free_list_corrupted;	/* corrupted free list */
int		bad_blocks;		/* num of bad blks seen (per inode) */
int		dup_blocks;		/* num of dup blks seen (per inode) */

char		*fi_name;		/* searching for this name */
unsigned short	fi_result;		/* result of inode search */
unsigned short	lost_inode;		/* lost file to reconnect */

unsigned long	scan_filesize;		/* file size, decremented during scan */
unsigned short	total_files;		/* number of files seen */

void scan_pass2 PARAMS((struct filesys*, unsigned int));

void
set_inode_state (inum, s)
	unsigned int inum;
	int s;
{
	unsigned int byte, shift;

	byte = inum / STATES_PER_BYTE;
	shift = inum % STATES_PER_BYTE * STATE_BITS;
	state_map [byte] &= ~(STATE_MASK << shift);
	state_map [byte] |= s << shift;
}

int
inode_state (inum)
	unsigned int inum;
{
	unsigned int byte, shift;

	byte = inum / STATES_PER_BYTE;
	shift = inum % STATES_PER_BYTE * STATE_BITS;
	return (state_map [byte] >> shift) & STATE_MASK;
}

int
block_is_busy (blk)
	unsigned int blk;
{
	return block_map [blk >> 3] & (1 << (blk & 7));
}

void
busy_block_mark (blk)
	unsigned int blk;
{
	block_map [blk >> 3] |= 1 << (blk & 7);
}

void
free_block_mark (blk)
	unsigned int blk;
{
	block_map [blk >> 3] &= ~(1 << (blk & 7));
}

void
free_list_mark (blk)
	unsigned int blk;
{
	free_map [blk >> 3] |= 1 << (blk & 7);
}

int
in_free_list (blk)
	unsigned int blk;
{
	return free_map [blk >> 3] & (1 << (blk & 7));
}

void
pr_io_error (s, blk)
	char *s;
	unsigned int blk;
{
	printf ("\nCAN NOT %s: BLK %d\n", s, blk);
}

void
buf_flush (fs)
	struct filesys *fs;
{
	if (buf_dirty && fs->writable) {
/*printf ("WRITE blk %d\n", buf_bno);*/
		if (! fs_bwrite (fs, buf_bno, buf_data))
			pr_io_error ("WRITE", buf_bno);
	}
	buf_dirty = 0;
}

int
buf_get (fs, blk)
	struct filesys *fs;
	unsigned int blk;
{
	if (buf_bno == blk)
		return 1;
	buf_flush (fs);
/*printf ("read blk %d\n", blk);*/
	if (! fs_bread (fs, blk, buf_data)) {
		pr_io_error ("READ", blk);
		buf_bno = (unsigned short)-1;
		return 0;
	}
	buf_bno = blk;
	return 1;
}

/*
 * Scan recursively the indirect block of the inode,
 * and for every block call the given function.
 */
int
scan_iblock (inode, blk, double_indirect, flg, func, arg)
	struct inode *inode;
	unsigned int blk;
	int double_indirect, flg;
	scanner_t *func;
	void *arg;
{
	unsigned int nb;
	int ret, i;
	unsigned char data [FS_BSIZE];

/*printf ("check %siblock %d: \n", double_indirect ? "double " : "", blk);*/
	if (flg == ADDR) {
		ret = (*func) (inode, blk, arg);
		if (! (ret & KEEPON))
			return ret;
	}
	if (outrange (inode->fs, blk))		/* protect thyself */
		return SKIP;

	if (! fs_bread (inode->fs, blk, data)) {
		pr_io_error ("READ", blk);
		return SKIP;
	}
	for (i = 0; i < FS_BSIZE; i+=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb) {
			if (double_indirect)
				ret = scan_iblock (inode, nb,
					0, flg, func, arg);
			else
				ret = (*func) (inode, nb, arg);

			if (ret & STOP)
				return ret;
		}
	}
	return KEEPON;
}

/*
 * Scan recursively the block list of the inode,
 * and for every block call the given function.
 * Option flg:
 * - when ADDR - call func for both data and indirect blocks
 * - when DATA - only data blocks are processed
 */
int
scan_inode (inode, flg, func, arg)
	struct inode *inode;
	int flg;
	scanner_t *func;
	void *arg;
{
	unsigned short *ap;
	int ret;

/*printf ("check inode %d: %#o\n", inode->number, inode->mode);*/
	if (S_ISBLK(inode->mode) || S_ISCHR(inode->mode))
		return KEEPON;
	scan_filesize = inode->size;

	if (! (inode->mode & S_LARGE)) {
		/* Small file - up to 8 direct blocks. */
		for (ap = inode->addr; ap < &inode->addr[8]; ap++) {
			if (*ap) {
				ret = (*func) (inode, *ap, arg);
				if (ret & STOP)
					return ret;
			}
		}
		return KEEPON;
	}
	/* Large file - up to 7 indirect blocks and
	 * one double indirect block. */
	for (ap = inode->addr; ap < &inode->addr[7]; ap++) {
		if (*ap) {
			ret = scan_iblock (inode, *ap, 0,
				flg, func, arg);
			if (ret & STOP)
				return (ret);
		}
	}
	if (inode->addr[7]) {
		/* Check the last (indirect) block. */
		ret = scan_iblock (inode, inode->addr[7], 1,
			flg, func, arg);
		if (ret & STOP)
			return (ret);
	}
	return KEEPON;
}

void
pr_block_error (s, blk, inum)
	char *s;
	unsigned int blk, inum;
{
	printf ("%u %s I=%u\n", blk, s, inum);
}

/*
 * Called once for every block of every file.
 * Mark blocks as busy on block map.
 * If duplicates are found, put them into dup_list.
 */
int
pass1 (inode, blk, arg)
	struct inode *inode;
	unsigned int blk;
	void *arg;
{
	unsigned short *dlp;
	unsigned short *blocks = (unsigned short*) arg;

/*printf ("pass1 inode %d block %d: \n", inode->number, blk);*/
	if (outrange (inode->fs, blk)) {
		pr_block_error ("BAD", blk, inode->number);
		set_inode_state (inode->number, CLEAR);	/* mark for possible clearing */
		if (++bad_blocks >= MAXBAD) {
			printf ("EXCESSIVE BAD BLKS I=%u\n", inode->number);
			return STOP;
		}
		return SKIP;
	}
	if (block_is_busy (blk)) {
		pr_block_error ("DUP", blk, inode->number);
		set_inode_state (inode->number, CLEAR);	/* mark for possible clearing */
		if (++dup_blocks >= MAXDUP) {
			printf ("EXCESSIVE DUP BLKS I=%u\n", inode->number);
			return STOP;
		}
		if (dup_end >= &dup_list[DUP_LIST_SIZE]) {
			printf ("DUP TABLE OVERFLOW.\n");
			return STOP;
		}
		for (dlp = dup_list; dlp < dup_multi; dlp++) {
			if (*dlp == blk) {
				*dup_end++ = blk;
				break;
			}
		}
		if (dlp >= dup_multi) {
			*dup_end++ = *dup_multi;
			*dup_multi++ = blk;
		}
	} else {
		if (blocks)
			++*blocks;
		busy_block_mark (blk);
	}
	return KEEPON;
}

int pass1b (inode, blk, arg)
	struct inode *inode;
	unsigned int blk;
	void *arg;
{
	unsigned short *dlp;

	if (outrange (inode->fs, blk))
		return SKIP;
	for (dlp = dup_list; dlp < dup_multi; dlp++) {
		if (*dlp == blk) {
			pr_block_error ("DUP", blk, inode->number);
			set_inode_state (inode->number, CLEAR);	/* mark for possible clearing */
			*dlp = *--dup_multi;
			*dup_multi = blk;
			return (dup_multi == dup_list ? STOP : KEEPON);
		}
	}
	return KEEPON;
}

/*
 * Read directory, and for every entry call given function.
 * If function altered the contents of entry, then write it back.
 */
int
scan_directory (inode, blk, arg)
	struct inode *inode;
	unsigned int blk;
	void *arg;
{
	struct dirent direntry;
	unsigned char *dirp;
	int (*func) () = (scanner_t*) arg;
	int n;

/*printf ("scan_directory: I=%d, blk=%d\n", inode->number, blk);*/
	if (outrange (inode->fs, blk)) {
		scan_filesize -= FS_BSIZE;
		return SKIP;
	}
	dirp = buf_data;
	while (dirp < &buf_data[FS_BSIZE] && scan_filesize > 0) {
		if (! buf_get (inode->fs, blk)) {
			scan_filesize -= (&buf_data[FS_BSIZE] - dirp);
			return SKIP;
		}

		/* For every directory entry, call handler. */
		n = (*func) (inode->fs, &direntry);

		if (n & ALTERD) {
			if (buf_get (inode->fs, blk)) {
				*(struct dirent*) dirp = direntry;
				buf_dirty = 1;
			} else
				n &= ~ALTERD;
		}
		if (n & STOP)
			return n;
		dirp += 16;
		scan_filesize -= 16;
	}
	return (scan_filesize > 0) ? KEEPON : STOP;
}

void
pr_inode (inode)
	struct inode *inode;
{
	char *p;

	printf (" I=%u ", inode->number);
	printf (" OWNER=%d ", inode->uid);
	printf ("MODE=%o\n", inode->mode);
	printf ("SIZE=%ld ", inode->size);
	p = ctime ((long*) &inode->mtime);
	printf ("MTIME=%12.12s %4.4s\n", p+4, p+20);
}

void
pr_dir_error (fs, inum, s)
	struct filesys *fs;
	unsigned int inum;
	char *s;
{
	struct inode inode;

	if (! fs_iget (fs, &inode, inum)) {
		printf ("%s  I=%u\nNAME=%s\n", s, inum, pathname);
		return;
	}
	printf ("%s ", s);
	pr_inode (&inode);
	printf ("%s=%s\n", S_ISDIR(inode.mode) ? "DIR" : "FILE", pathname);
}

/*
 * Clear directory entries which refer to duplicated or unallocated inodes.
 * Decrement link counters.
 */
int
pass2 (fs, dirp)
	struct filesys *fs;
	struct dirent *dirp;
{
	int inum, n, ret = KEEPON;
	struct inode inode;

	inum = dirp->d_ino;
	if (inum == 0)
		return KEEPON;

	/* Copy file name from dirp to pathp */
	thisname = pathp;
	strcpy (pathp, dirp->d_name);
	pathp += strlen (pathp);
/*printf ("%s  %d\n", pathname, inum);*/
	n = 0;
	if (inum > fs->isize * FS_INODES_PER_BLOCK ||
	    inum < FS_ROOT_INODE)
		pr_dir_error (fs, inum, "I OUT OF RANGE");
	else {
again:		switch (inode_state (inum)) {
		case USTATE:
			pr_dir_error (fs, inum, "UNALLOCATED");
			if (fs->writable) {
				dirp->d_ino = 0;
				ret |= ALTERD;
				break;
			}
			break;
		case CLEAR:
			pr_dir_error (fs, inum, "DUP/BAD");
			if (fs->writable) {
				dirp->d_ino = 0;
				ret |= ALTERD;
				break;
			}
			if (! fs_iget (fs, &inode, inum))
				break;
			set_inode_state (inum, S_ISDIR(inode.mode) ?
				DSTATE : FSTATE);
			goto again;
		case FSTATE:
			--link_count [inum];
			break;
		case DSTATE:
			--link_count [inum];
			scan_pass2 (fs, inum);
		}
	}
	pathp = thisname;
	return ret;
}

/*
 * Traverse directory tree. Call pass2 for every directory entry.
 * Keep current file name in 'pathname'.
 */
void
scan_pass2 (fs, inum)
	struct filesys *fs;
	unsigned int inum;
{
	struct inode inode;
	char *savname;
	unsigned long savsize;

	set_inode_state (inum, FSTATE);
	if (! fs_iget (fs, &inode, inum))
		return;
	*pathp++ = '/';
	savname = thisname;
	savsize = scan_filesize;
	scan_inode (&inode, DATA, scan_directory, pass2);
	scan_filesize = savsize;
	thisname = savname;
	*--pathp = 0;
}

/*
 * Find inode number by name.
 * The name is in 'fi_name'.
 * Put resulting inode number into 'fi_result'.
 */
int
find_inode (fs, dirp)
	struct filesys *fs;
	struct dirent *dirp;
{
	if (dirp->d_ino == 0)
		return KEEPON;
	if (strcmp (fi_name, dirp->d_name) == 0) {
		if (dirp->d_ino >= FS_ROOT_INODE &&
		    dirp->d_ino <= fs->isize * FS_INODES_PER_BLOCK)
			fi_result = dirp->d_ino;
		return STOP;
	}
	return KEEPON;
}

/*
 * Find a free slot and make link to 'lost_inode'.
 * Create filename of a kind "#01234".
 */
int
make_lost_entry (fs, dirp)
	struct filesys *fs;
	struct dirent *dirp;
{
	int n;
	char *p;

	if (dirp->d_ino)
		return KEEPON;
	dirp->d_ino = lost_inode;
	p = dirp->d_name + 6;
	*p-- = 0;
	for (n = dirp->d_ino; p > dirp->d_name; n /= 10)
		*p-- = '0' + (n % 10);
	*p = '#';
	return ALTERD | STOP;
}

/*
 * For entry ".." set inode number to 'lost_found_inode'.
 */
int
dotdot_to_lost_found (fs, dirp)
	struct filesys *fs;
	struct dirent *dirp;
{
	if (dirp->d_name[0] == '.' && dirp->d_name[1] == '.' &&
	    dirp->d_name[2] == 0) {
		dirp->d_ino = lost_found_inode;
		return ALTERD | STOP;
	}
	return KEEPON;
}

/*
 * Return lost+found inode number.
 * TODO: create /lost+found when not available.
 */
unsigned int
find_lost_found (fs)
	struct filesys *fs;
{
	struct inode root;

	/* Find lost_found directory inode number. */
	if (! fs_iget (fs, &root, FS_ROOT_INODE))
		return 0;
	fi_name = lost_found_name;
	fi_result = 0;
	scan_inode (&root, DATA, scan_directory, find_inode);
	return fi_result;
}

/*
 * Restore a link to parent directory - "..".
 */
int
move_to_lost_found (inode)
	struct inode *inode;
{
	struct inode lost_found;

	printf ("UNREF %s ", S_ISDIR(inode->mode) ? "DIR" : "FILE");
	pr_inode (inode);
	if (! inode->fs->writable)
		return 0;

	/* Get lost+found inode. */
	if (lost_found_inode == 0) {
		/* Find lost_found directory inode number. */
		lost_found_inode = find_lost_found (inode->fs);
		if (! lost_found_inode) {
			printf ("SORRY. NO lost+found DIRECTORY\n\n");
			return 0;
		}
	}
	if (! fs_iget (inode->fs, &lost_found, lost_found_inode) ||
	    ! S_ISDIR(lost_found.mode) ||
	    inode_state (lost_found_inode) != FSTATE) {
		printf ("SORRY. NO lost+found DIRECTORY\n\n");
		return 0;
	}
	if (lost_found.size % FS_BSIZE) {
		lost_found.size = (lost_found.size + FS_BSIZE - 1) /
			FS_BSIZE * FS_BSIZE;
		if (! fs_isave (&lost_found, 1)) {
			printf ("SORRY. ERROR WRITING lost+found I-NODE\n\n");
			return 0;
		}
	}

	/* Put a file to lost+found. */
	lost_inode = inode->number;
	if ((scan_inode (&lost_found, DATA, scan_directory, make_lost_entry) &
	    ALTERD) == 0) {
		printf ("SORRY. NO SPACE IN lost+found DIRECTORY\n\n");
		return 0;
	}
	--link_count [inode->number];

	if (S_ISDIR(inode->mode)) {
		/* For ".." set inode number to lost_found_inode. */
		scan_inode (inode, DATA, scan_directory, dotdot_to_lost_found);
		if (fs_iget (inode->fs, &lost_found, lost_found_inode)) {
			lost_found.nlink++;
			++link_count [lost_found.number];
			if (! fs_isave (&lost_found, 1)) {
				printf ("SORRY. ERROR WRITING lost+found I-NODE\n\n");
				return 0;
			}
		}
		printf ("DIR I=%u CONNECTED.\n\n", inode->number);
	}
	return 1;
}

/*
 * Mark the block as free. Remove it from dup list.
 */
int
pass4 (inode, blk, arg)
	struct inode *inode;
	unsigned int blk;
	void *arg;
{
	unsigned short *dlp;
	unsigned short *blocks = (unsigned short*) arg;

	if (outrange (inode->fs, blk))
		return SKIP;
	if (block_is_busy (blk)) {
		/* Free block. */
		for (dlp = dup_list; dlp < dup_end; dlp++)
			if (*dlp == blk) {
				*dlp = *--dup_end;
				return KEEPON;
			}
		free_block_mark (blk);
		if (blocks)
			--*blocks;
	}
	return KEEPON;
}

/*
 * Clear the inode, mark it's blocks as free.
 */
void
clear_inode (fs, inum, msg)
	struct filesys *fs;
	unsigned int inum;
	char *msg;
{
	struct inode inode;

	if (! fs_iget (fs, &inode, inum))
		return;
	if (msg) {
		printf ("%s %s", msg, S_ISDIR(inode.mode) ? "DIR" : "FILE");
		pr_inode (&inode);
	}
	if (fs->writable) {
		total_files--;
		scan_inode (&inode, ADDR, pass4, 0);
		fs_iclear (&inode);
		fs_isave (&inode, 1);
	}
}

/*
 * Fix the link count of the inode.
 * If no links - move it to lost+found.
 */
void
adjust_link_count (fs, inum, lcnt)
	struct filesys *fs;
	unsigned int inum;
	short lcnt;
{
	struct inode inode;

	if (! fs_iget (fs, &inode, inum))
		return;
	if (inode.nlink == lcnt) {
		/* No links to file - move to lost+found. */
		if (! move_to_lost_found (&inode))
			clear_inode (fs, inum, 0);
	} else {
		printf ("LINK COUNT %s", (lost_found_inode==inum) ? lost_found_name :
			S_ISDIR(inode.mode) ? "DIR" : "FILE");
		pr_inode (&inode);
		printf ("COUNT %d SHOULD BE %d\n",
			inode.nlink, inode.nlink - lcnt);
		if (fs->writable) {
			inode.nlink -= lcnt;
			fs_isave (&inode, 1);
		}
	}
}

/*
 * Called from chk_free_list() for every block in free list.
 * Count free blocks, detect duplicates.
 */
int
pass5 (fs, blk, free_blocks)
	struct filesys *fs;
	unsigned int blk;
	unsigned short *free_blocks;
{
	if (outrange (fs, blk)) {
		free_list_corrupted = 1;
		if (++bad_blocks >= MAXBAD) {
			printf ("EXCESSIVE BAD BLKS IN FREE LIST.\n");
			return STOP;
		}
		return SKIP;
	}
	if (in_free_list (blk)) {
		free_list_corrupted = 1;
		if (++dup_blocks >= DUP_LIST_SIZE) {
			printf ("EXCESSIVE DUP BLKS IN FREE LIST.\n");
			return STOP;
		}
	} else {
		++*free_blocks;
		free_list_mark (blk);
	}
	return KEEPON;
}

/*
 * Scan a free block list and return a number of free blocks.
 * If the list is corrupted, set 'free_list_corrupted' flag.
 */
unsigned int
chk_free_list (fs)
	struct filesys *fs;
{
	unsigned short *ap, *base;
	unsigned int free_blocks, nfree;
	unsigned short list [100];
	unsigned short *data = (unsigned short*) buf_data;
	int i;

	if (fs->nfree == 0)
		return 0;
	free_blocks = 0;
	nfree = fs->nfree;
	base = fs->free;
	for (;;) {
		if (nfree <= 0 || nfree > 100) {
			printf ("BAD FREEBLK COUNT\n");
			free_list_corrupted = 1;
			break;
		}
		ap = base + nfree;
		while (--ap > base) {
			if (pass5 (fs, *ap, &free_blocks) == STOP)
				return free_blocks;
		}
		if (*ap == 0 || pass5 (fs, *ap, &free_blocks) != KEEPON)
			break;
		if (! fs_bread (fs, *ap, (void*) data)) {
			pr_io_error ("READ", *ap);
			break;
		}
		nfree = data[0];
		for (i=0; i<100; ++i)
			list [i] = data[i+1];
		base = list;
	}
	return free_blocks;
}

/*
 * Check a list of free inodes.
 */
void
chk_ifree_list (fs)
	struct filesys *fs;
{
	int i;
	unsigned int inum;

	for (i=0; i<fs->ninode; i++) {
		inum = fs->inode[i];
		if (inode_state (inum) != USTATE) {
			printf ("ALLOCATED INODE(S) IN IFREE LIST\n");
			if (fs->writable) {
				fs->ninode = i - 1;
				while (i < 100)
					fs->inode [i++] = 0;
				fs->dirty = 1;
			}
			return;
		}
	}
}

/*
 * Build a free block list from scratch.
 */
unsigned int
make_free_list (fs)
	struct filesys *fs;
{
	unsigned int free_blocks, n;

	fs->nfree = 0;
	fs->flock = 0;
	fs->fmod = 0;
	fs->ilock = 0;
	fs->ronly = 0;
	fs->dirty = 1;
	free_blocks = 0;

	/* Build a list of free blocks */
	fs_bfree (fs, 0);
	for (n = fs->fsize - 1; n >= fs->isize + 2; n--) {
		if (block_is_busy (n))
			continue;
		++free_blocks;
		if (! fs_bfree (fs, n))
			return 0;
	}
	return free_blocks;
}

/*
 * Check filesystem for errors.
 * When readonly - just check and print errors.
 * If the system is open on read/write - fix errors.
 */
int
fsck (fs)
	struct filesys *fs;
{
	struct inode inode;
	int n;
	unsigned int inum;
	unsigned int block_map_size;		/* number of free blocks */
	unsigned int free_blocks;		/* number of free blocks */
	unsigned int used_blocks;		/* number of blocks used */
	unsigned int last_allocated_inode;	/* hiwater mark of inodes */

	if (fs->isize + 2 >= fs->fsize) {
		printf ("Bad filesystem size: total %d blocks with %d inode blocks\n",
			fs->fsize, fs->isize);
		return 0;
	}
	free_list_corrupted = 0;
	total_files = 0;
	used_blocks = 0;
	dup_multi = dup_end = &dup_list[0];
	end_bad_link_list = &bad_link_list[0];
	lost_found_inode = 0;
	buf_dirty = 0;
	buf_bno = (unsigned short) -1;

	/* Allocate memory. */
	block_map_size = (fs->fsize + 7) / 8;
	block_map = calloc (block_map_size, sizeof (*block_map));
	state_map = calloc ((fs->isize * FS_INODES_PER_BLOCK +
		STATES_PER_BYTE) / STATES_PER_BYTE, sizeof (*state_map));
	link_count = (short*) calloc (fs->isize * FS_INODES_PER_BLOCK + 1,
		sizeof (*link_count));
	if (! block_map || ! state_map || ! link_count) {
		printf ("Cannot allocate memory\n");
fatal:		if (block_map)
			free (block_map);
		if (state_map)
			free (state_map);
		if (link_count)
			free ((char*) link_count);
		return 0;
	}

	printf ("** Phase 1 - Check Blocks and Sizes\n");
	last_allocated_inode = 0;
	for (inum = 1; inum <= fs->isize * FS_INODES_PER_BLOCK; inum++) {
		if (! fs_iget (fs, &inode, inum))
			continue;
		if (inode.mode & S_ALLOC) {
/*printf ("inode %d: %#o\n", inode.number, inode.mode);*/
			last_allocated_inode = inum;
			total_files++;
			link_count[inum] = inode.nlink;
			if (link_count[inum] <= 0) {
				if (end_bad_link_list < &bad_link_list[LINK_LIST_SIZE])
					*end_bad_link_list++ = inum;
				else {
					printf ("LINK COUNT TABLE OVERFLOW\n");
				}
			}
			set_inode_state (inum, S_ISDIR(inode.mode) ?
				DSTATE : FSTATE);
			bad_blocks = dup_blocks = 0;
			scan_inode (&inode, ADDR, pass1, &used_blocks);
			n = inode_state (inum);
			if (n == DSTATE || n == FSTATE) {
				if (S_ISDIR(inode.mode) &&
				    (inode.size % 16) != 0) {
					printf ("DIRECTORY MISALIGNED I=%u\n\n",
						inode.number);
				}
			}
		}
		else if (inode.mode != 0) {
			printf ("PARTIALLY ALLOCATED INODE I=%u\n", inum);
			if (fs->writable)
				fs_iclear (&inode);
		}
		fs_isave (&inode, 0);
	}
	if (dup_end != &dup_list[0]) {
		printf ("** Phase 1b - Rescan For More DUPS\n");
		for (inum = 1; inum <= last_allocated_inode; inum++) {
			if (inode_state (inum) == USTATE)
				continue;
			if (! fs_iget (fs, &inode, inum))
				continue;
			if (scan_inode (&inode, ADDR, pass1b, 0) & STOP)
				break;
		}
	}

	printf ("** Phase 2 - Check Pathnames\n");
	thisname = pathp = pathname;
	switch (inode_state (FS_ROOT_INODE)) {
	case USTATE:
		printf ("ROOT INODE UNALLOCATED. TERMINATING.\n");
		goto fatal;
	case FSTATE:
		printf ("ROOT INODE NOT DIRECTORY\n");
		if (! fs->writable)
			goto fatal;
		if (! fs_iget (fs, &inode, FS_ROOT_INODE))
			goto fatal;
		inode.mode &= ~S_IFMT;
		inode.mode |= S_IFDIR;
		fs_isave (&inode, 1);
		set_inode_state (FS_ROOT_INODE, DSTATE);
	case DSTATE:
		scan_pass2 (fs, FS_ROOT_INODE);
		break;
	case CLEAR:
		printf ("DUPS/BAD IN ROOT INODE\n");
		set_inode_state (FS_ROOT_INODE, DSTATE);
		scan_pass2 (fs, FS_ROOT_INODE);
	}

	printf ("** Phase 3 - Check Connectivity\n");
	for (inum = FS_ROOT_INODE; inum <= last_allocated_inode; inum++) {
		if (inode_state (inum) == DSTATE) {
			unsigned int ino;

			fi_name = "..";
			ino = inum;
			do {
				if (! fs_iget (fs, &inode, ino))
					break;
				fi_result = 0;
				scan_inode (&inode, DATA, scan_directory, find_inode);
				if (fi_result == 0) {
					/* Parent link lost. */
					if (move_to_lost_found (&inode)) {
						thisname = pathp = pathname;
						*pathp++ = '?';
						scan_pass2 (fs, ino);
					}
					break;
				}
				ino = fi_result;
			} while (inode_state (ino) == DSTATE);
		}
	}

	printf ("** Phase 4 - Check Reference Counts\n");
	for (inum = FS_ROOT_INODE; inum <= last_allocated_inode; inum++) {
		switch (inode_state (inum)) {
		case FSTATE:
			n = link_count [inum];
			if (n)
				adjust_link_count (fs, inum, n);
			else {
				unsigned short *blp;

				for (blp = bad_link_list; blp < end_bad_link_list; blp++)
					if (*blp == inum) {
						clear_inode (fs, inum, "UNREF");
						break;
					}
			}
			break;
		case DSTATE:
			clear_inode (fs, inum, "UNREF");
			break;
		case CLEAR:
			clear_inode (fs, inum, "BAD/DUP");
		}
	}
	buf_flush (fs);

	printf ("** Phase 5 - Check Free List\n");
	free ((char*) link_count);
	chk_ifree_list (fs);
	free (state_map);
	bad_blocks = dup_blocks = 0;
	free_map = calloc (block_map_size, sizeof (*free_map));
	if (! free_map) {
		printf ("NO MEMORY TO CHECK FREE LIST\n");
		free_list_corrupted = 1;
		free_blocks = 0;
	} else {
		memcpy (free_map, block_map, block_map_size);
		free_blocks = chk_free_list (fs);
		free (free_map);
	}
	if (bad_blocks)
		printf ("%d BAD BLKS IN FREE LIST\n", bad_blocks);
	if (dup_blocks)
		printf ("%d DUP BLKS IN FREE LIST\n", dup_blocks);
	if (free_list_corrupted == 0) {
		if (used_blocks + free_blocks != fs->fsize - fs->isize - 2) {
			printf ("%d BLK(S) MISSING\n", fs->fsize -
				fs->isize - 2 - used_blocks - free_blocks);
			free_list_corrupted = 1;
		}
	}
	if (free_list_corrupted) {
		printf ("BAD FREE LIST\n");
		if (! fs->writable)
			free_list_corrupted = 0;
	}

	if (free_list_corrupted) {
		printf ("** Phase 6 - Salvage Free List\n");
		free_blocks = make_free_list (fs);
	}

	printf ("%d files %d blocks %d free\n",
		total_files, used_blocks, free_blocks);
	if (fs->modified) {
		time ((long*) &fs->time);
		fs->dirty = 1;
	}
	buf_flush (fs);
	fs_sync (fs, 0);
	if (fs->modified)
		printf ("\n***** FILE SYSTEM WAS MODIFIED *****\n");

	free (block_map);
	return 1;
}

int
main (argc, argv)
	int argc;
	char **argv;
{
	int i, fix;
	char *p;
	static struct filesys fs;

	fix = 0;
	argv++;
	if (argc > 1) {
		p = *argv;
		if (*p++ == '-') {
			while ((i = *p++) != '\0') {
				switch (i) {
				case 'f':
					fix = 1;
					break;
				}
			}
			argc--;
			argv++;
		}
	}
	if (argc != 2) {
		printf ("Usage: fsck [-f] device\n");
		return 1;
	}
	/* Check filesystem for errors, and optionally fix them. */
	if (! fs_open (&fs, *argv, fix)) {
		printf ("%s: cannot open\n", *argv);
		return -1;
	}
	fsck (&fs);
	fs_close (&fs);
	return 0;
}

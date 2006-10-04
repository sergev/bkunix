#define FS_BSIZE		512	/* block size */
#define FS_ROOT_INODE		1	/* root directory in inode 1 */
#define FS_INODES_PER_BLOCK	16	/* inodes per block */

struct filesys {
	unsigned short	isize;		/* size in blocks of I list */
	unsigned short	fsize;		/* size in blocks of entire volume */
	unsigned short	nfree;		/* number of in core free blocks (0-100) */
	unsigned short	free [100];	/* in core free blocks */
	unsigned short	ninode;		/* number of in core I nodes (0-100) */
	unsigned short	inode [100];	/* in core free I nodes */
	unsigned char	flock;		/* lock during free list manipulation */
	unsigned char	ilock;		/* lock during I list manipulation */
	unsigned char	fmod;		/* super block modified flag */
	unsigned char	ronly;		/* mounted read-only flag */
	unsigned long	time;		/* current date of last update */

	const char	*filename;
	int		fd;
	unsigned long	seek;
	int		writable;
	int		dirty;		/* sync needed */
	int		modified;	/* write_block was called */
};
#define FS_SUPERB_SIZE	(2*208)		/* superblock size */

struct inode {
	unsigned short	mode;		/* file type and access mode */
	unsigned char	nlink;		/* directory entries */
	unsigned char	uid;		/* owner */
	unsigned char	gid;		/* owner */
	unsigned char	size2;		/* size msb */
	unsigned short	size10;		/* size lsw */
	unsigned short	addr [8];	/* device addresses constituting file */
	unsigned long	atime;		/* last access time */
	unsigned long	mtime;		/* last modification time */

	struct filesys	*fs;
	unsigned short	number;
	unsigned long	size;		/* size */
	int		dirty;		/* save needed */
};
#define FS_INODE_SIZE	32		/* inode size */

int fs_open PARAMS((struct filesys*, char*, int));
void fs_close PARAMS((struct filesys*));
int fs_sync PARAMS((struct filesys*, int));
void fs_print PARAMS((struct filesys*, int));

int fs_seek PARAMS((struct filesys*, unsigned long));
int fs_read PARAMS((struct filesys*, unsigned char*, int));
int fs_write PARAMS((struct filesys*, unsigned char*, int));

int fs_iget PARAMS((struct filesys*, struct inode*, unsigned short));
int fs_isave PARAMS((struct inode*, int));

int fs_bwrite PARAMS((struct filesys*, unsigned short, unsigned char*));
int fs_bread PARAMS((struct filesys*, unsigned short, unsigned char*));
int fs_bfree PARAMS((struct filesys*, unsigned int));
int fs_balloc PARAMS((struct filesys*, unsigned int*));
int fs_ibfree PARAMS((struct filesys*, unsigned int));
int fs_dibfree PARAMS((struct filesys*, unsigned int));

#if 0
int fs_check PARAMS((struct filesys*);

void fs_inode_clear PARAMS((struct inode *inode);
void fs_inode_truncate PARAMS((struct inode *inode);
void fs_inode_print PARAMS((struct inode *inode, FILE *out);
int fs_inode_read PARAMS((struct inode *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int fs_inode_write PARAMS((struct inode *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int fs_inode_alloc PARAMS((struct filesys*, struct inode *inode);
int fs_inode_by_name PARAMS((struct filesys*, struct inode *inode, char *name,
	int op, int mode);

void fs_directory_scan PARAMS((struct inode *inode, char *dirname,
	lsxfs_directory_scanner_t scanner, void *arg);
void fs_dirent_pack PARAMS((unsigned char *data, lsxfs_dirent_t *dirent);
void fs_dirent_unpack PARAMS((lsxfs_dirent_t *dirent, unsigned char *data);

int fs_file_create PARAMS((struct filesys*, lsxfs_file_t *file, char *name, int mode);
int fs_file_open PARAMS((struct filesys*, lsxfs_file_t *file, char *name, int wflag);
int fs_file_read PARAMS((lsxfs_file_t *file, unsigned char *data,
	unsigned long bytes);
int fs_file_write PARAMS((lsxfs_file_t *file, unsigned char *data,
	unsigned long bytes);
int fs_file_close PARAMS((lsxfs_file_t *file);

#endif

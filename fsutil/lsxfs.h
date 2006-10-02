#define LSXFS_BSIZE		512	/* block size */
#define LSXFS_ROOT_INODE	1	/* root directory in inode 1 */
#define LSXFS_INODES_PER_BLOCK	16	/* inodes per block */

typedef struct {
	const char	*filename;
	int		fd;
	unsigned long	seek;
	int		writable;
	int		dirty;		/* sync needed */
	int		modified;	/* write_block was called */

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
} lsxfs_t;

typedef struct {
	lsxfs_t		*fs;
	unsigned short	number;
	int		dirty;		/* save needed */

	unsigned short	mode;		/* file type and access mode */
#define	INODE_MODE_ALLOC	0100000
#define	INODE_MODE_FMT		060000
#define	INODE_MODE_FDIR		040000
#define	INODE_MODE_FCHR		020000
#define	INODE_MODE_FBLK		060000
#define	INODE_MODE_LARG		010000
#define	INODE_MODE_SUID		04000
#define	INODE_MODE_SGID		02000
#define	INODE_MODE_SVTX		01000
#define	INODE_MODE_READ		0400
#define	INODE_MODE_WRITE	0200
#define	INODE_MODE_EXEC		0100

	unsigned char	nlink;		/* directory entries */
	unsigned char	uid;		/* owner */
	unsigned char	gid;		/* group of owner */
	unsigned long	size;		/* size */
	unsigned short	addr [8];	/* device addresses constituting file */
	unsigned long	atime;		/* last access time */
	unsigned long	mtime;		/* last modification time */
} lsxfs_inode_t;

typedef struct {
	unsigned short	ino;
	char		name [14+1];
} lsxfs_dirent_t;

typedef void (*lsxfs_directory_scanner_t) (lsxfs_inode_t *dir,
	lsxfs_inode_t *file, char *dirname, char *filename, void *arg);

typedef struct {
	lsxfs_inode_t	inode;
	int		writable;	/* write allowed */
	unsigned long	offset;		/* current i/o offset */
} lsxfs_file_t;

int lsxfs_seek (lsxfs_t *fs, unsigned long offset);
int lsxfs_read8 (lsxfs_t *fs, unsigned char *val);
int lsxfs_read16 (lsxfs_t *fs, unsigned short *val);
int lsxfs_read32 (lsxfs_t *fs, unsigned long *val);
int lsxfs_write8 (lsxfs_t *fs, unsigned char val);
int lsxfs_write16 (lsxfs_t *fs, unsigned short val);
int lsxfs_write32 (lsxfs_t *fs, unsigned long val);

int lsxfs_read (lsxfs_t *fs, unsigned char *data, int bytes);
int lsxfs_write (lsxfs_t *fs, unsigned char *data, int bytes);

int lsxfs_open (lsxfs_t *fs, const char *filename, int writable);
void lsxfs_close (lsxfs_t *fs);
int lsxfs_sync (lsxfs_t *fs, int force);
int lsxfs_create (lsxfs_t *fs, const char *filename, unsigned long bytes);
int lsxfs_install_boot (lsxfs_t *fs, const char *filename,
	const char *filename2);
int lsxfs_install_single_boot (lsxfs_t *fs, const char *filename);
int lsxfs_check (lsxfs_t *fs);
void lsxfs_print (lsxfs_t *fs, FILE *out);

int lsxfs_inode_get (lsxfs_t *fs, lsxfs_inode_t *inode, unsigned short inum);
int lsxfs_inode_save (lsxfs_inode_t *inode, int force);
void lsxfs_inode_clear (lsxfs_inode_t *inode);
void lsxfs_inode_truncate (lsxfs_inode_t *inode);
void lsxfs_inode_print (lsxfs_inode_t *inode, FILE *out);
int lsxfs_inode_read (lsxfs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int lsxfs_inode_write (lsxfs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int lsxfs_inode_alloc (lsxfs_t *fs, lsxfs_inode_t *inode);
int lsxfs_inode_by_name (lsxfs_t *fs, lsxfs_inode_t *inode, char *name,
	int op, int mode);

int lsxfs_write_block (lsxfs_t *fs, unsigned short bnum, unsigned char *data);
int lsxfs_read_block (lsxfs_t *fs, unsigned short bnum, unsigned char *data);
int lsxfs_block_free (lsxfs_t *fs, unsigned int bno);
int lsxfs_block_alloc (lsxfs_t *fs, unsigned int *bno);
int lsxfs_indirect_block_free (lsxfs_t *fs, unsigned int bno);
int lsxfs_double_indirect_block_free (lsxfs_t *fs, unsigned int bno);

void lsxfs_directory_scan (lsxfs_inode_t *inode, char *dirname,
	lsxfs_directory_scanner_t scanner, void *arg);
void lsxfs_dirent_pack (unsigned char *data, lsxfs_dirent_t *dirent);
void lsxfs_dirent_unpack (lsxfs_dirent_t *dirent, unsigned char *data);

int lsxfs_file_create (lsxfs_t *fs, lsxfs_file_t *file, char *name, int mode);
int lsxfs_file_open (lsxfs_t *fs, lsxfs_file_t *file, char *name, int wflag);
int lsxfs_file_read (lsxfs_file_t *file, unsigned char *data,
	unsigned long bytes);
int lsxfs_file_write (lsxfs_file_t *file, unsigned char *data,
	unsigned long bytes);
int lsxfs_file_close (lsxfs_file_t *file);

/* Big endians: Motorola 68000, PowerPC, HP PA, IBM S390. */
#if defined (__m68k__) || defined (__ppc__) || defined (__hppa__) || \
    defined (__s390__)
#define lsx_short(x) ((x) << 8 | (unsigned char) ((x) >> 8))
#else
#define lsx_short(x) (x)
#endif

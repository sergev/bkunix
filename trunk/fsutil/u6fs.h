/*
 * Data structures for unix v6 filesystem.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
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
} u6fs_t;

typedef struct {
	u6fs_t		*fs;
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
	unsigned long	size;		/* size */
	unsigned short	addr [8];	/* device addresses constituting file */
	unsigned long	atime;		/* last access time */
	unsigned long	mtime;		/* last modification time */
} u6fs_inode_t;

typedef struct {
	unsigned short	ino;
	char		name [14+1];
} u6fs_dirent_t;

typedef void (*u6fs_directory_scanner_t) (u6fs_inode_t *dir,
	u6fs_inode_t *file, char *dirname, char *filename, void *arg);

typedef struct {
	u6fs_inode_t	inode;
	int		writable;	/* write allowed */
	unsigned long	offset;		/* current i/o offset */
} u6fs_file_t;

int u6fs_seek (u6fs_t *fs, unsigned long offset);
int u6fs_read8 (u6fs_t *fs, unsigned char *val);
int u6fs_read16 (u6fs_t *fs, unsigned short *val);
int u6fs_read32 (u6fs_t *fs, unsigned long *val);
int u6fs_write8 (u6fs_t *fs, unsigned char val);
int u6fs_write16 (u6fs_t *fs, unsigned short val);
int u6fs_write32 (u6fs_t *fs, unsigned long val);

int u6fs_read (u6fs_t *fs, unsigned char *data, int bytes);
int u6fs_write (u6fs_t *fs, unsigned char *data, int bytes);

int u6fs_open (u6fs_t *fs, const char *filename, int writable);
void u6fs_close (u6fs_t *fs);
int u6fs_sync (u6fs_t *fs, int force);
int u6fs_create (u6fs_t *fs, const char *filename, unsigned long bytes);
int u6fs_install_boot (u6fs_t *fs, const char *filename,
	const char *filename2);
int u6fs_install_single_boot (u6fs_t *fs, const char *filename);
int u6fs_check (u6fs_t *fs);
void u6fs_print (u6fs_t *fs, FILE *out);

int u6fs_inode_get (u6fs_t *fs, u6fs_inode_t *inode, unsigned short inum);
int u6fs_inode_save (u6fs_inode_t *inode, int force);
void u6fs_inode_clear (u6fs_inode_t *inode);
void u6fs_inode_truncate (u6fs_inode_t *inode);
void u6fs_inode_print (u6fs_inode_t *inode, FILE *out);
int u6fs_inode_read (u6fs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int u6fs_inode_write (u6fs_inode_t *inode, unsigned long offset,
	unsigned char *data, unsigned long bytes);
int u6fs_inode_alloc (u6fs_t *fs, u6fs_inode_t *inode);
int u6fs_inode_by_name (u6fs_t *fs, u6fs_inode_t *inode, char *name,
	int op, int mode);

int u6fs_write_block (u6fs_t *fs, unsigned short bnum, unsigned char *data);
int u6fs_read_block (u6fs_t *fs, unsigned short bnum, unsigned char *data);
int u6fs_block_free (u6fs_t *fs, unsigned int bno);
int u6fs_block_alloc (u6fs_t *fs, unsigned int *bno);
int u6fs_indirect_block_free (u6fs_t *fs, unsigned int bno);
int u6fs_double_indirect_block_free (u6fs_t *fs, unsigned int bno);

void u6fs_directory_scan (u6fs_inode_t *inode, char *dirname,
	u6fs_directory_scanner_t scanner, void *arg);
void u6fs_dirent_pack (unsigned char *data, u6fs_dirent_t *dirent);
void u6fs_dirent_unpack (u6fs_dirent_t *dirent, unsigned char *data);

int u6fs_file_create (u6fs_t *fs, u6fs_file_t *file, char *name, int mode);
int u6fs_file_open (u6fs_t *fs, u6fs_file_t *file, char *name, int wflag);
int u6fs_file_read (u6fs_file_t *file, unsigned char *data,
	unsigned long bytes);
int u6fs_file_write (u6fs_file_t *file, unsigned char *data,
	unsigned long bytes);
int u6fs_file_close (u6fs_file_t *file);

/* Big endians: Motorola 68000, PowerPC, HP PA, IBM S390. */
#if defined (__m68k__) || defined (__ppc__) || defined (__hppa__) || \
    defined (__s390__)
#define lsb_short(x) ((x) << 8 | (unsigned char) ((x) >> 8))
#else
#define lsb_short(x) (x)
#endif

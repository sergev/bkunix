#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_ 1

struct stat {
	int		st_dev;
	int		st_ino;
	int		st_mode;
	char		st_nlink;
	char		st_uid;
	char		st_gid;
	char		st_size0;
	unsigned	st_size;
	int		st_addr[8];
	long		st_atime;
	long		st_mtime;
};

#define	S_IFMT	0060000		/* type of file */
#define		S_IFDIR	0040000	/* directory */
#define		S_IFCHR	0020000	/* character special */
#define		S_IFBLK	0060000	/* block special */
#define		S_IFREG	0000000	/* regular */
#define	S_LARGE	0010000
#define	S_ISUID	0004000		/* set user id on execution */
#define	S_ISGID	0002000		/* set group id on execution */
#define	S_ISVTX	0001000		/* save swapped text even after use */

#define S_IRWXU 0000700		/* RWX mask for owner */
#define S_IRUSR 0000400		/* R for owner */
#define S_IWUSR 0000200		/* W for owner */
#define S_IXUSR 0000100		/* X for owner */

#define S_IRWXG 0000070		/* RWX mask for group */
#define S_IRGRP 0000040		/* R for group */
#define S_IWGRP 0000020		/* W for group */
#define S_IXGRP 0000010		/* X for group */

#define S_IRWXO 0000007		/* RWX mask for other */
#define S_IROTH 0000004		/* R for other */
#define S_IWOTH 0000002		/* W for other */
#define S_IXOTH 0000001		/* X for other */

#define	S_ISDIR(m)  	(((m) & S_IFMT) == S_IFDIR)	/* directory */
#define	S_ISCHR(m)  	(((m) & S_IFMT) == S_IFCHR)	/* char special */
#define	S_ISBLK(m)  	(((m) & S_IFMT) == S_IFBLK)	/* block special */
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	/* regular file */

#endif /* _SYS_STAT_H_ */

#define	ARCMAGIC 0177545

struct ar_hdr {
	char ar_name[14];		/* name */
	long ar_date;			/* modification time */
	char ar_uid;			/* user id */
	char ar_gid;			/* group id */
	int ar_mode;			/* octal file permissions */
	long ar_size;			/* size in bytes */
};

#define AR_HDRSIZE	(14 + 4 + 1 + 1 + 2 + 4)

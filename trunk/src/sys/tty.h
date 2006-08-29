/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and 6 characters.
 * The routines getc and putc (mch.s)
 * manipulate these structures.
 */
struct clist
{
	int	c_cc;		/* character count */
	int	c_cf;		/* pointer to first block */
	int	c_cl;		/* pointer to last block */
};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct tty
{
	struct	clist t_rawq;	/* input chars right off device */
	struct	clist t_canq;	/* input chars after erase and kill */
	struct	clist t_outq;	/* output list to device */
	char	t_delct;	/* number of delimiters in raw q */
	char	t_col;		/* printing column of device */
	char t_flags;		/* see below */
	char t_modes;		/* open or not? */
};

#define	TTIPRI	10
#define	TTOPRI	20

#define	CERASE	'#'		/* default special characters */
#define	CEOT	004
#define	CKILL	'@'
#define	CQUIT	034		/* FS, cntl shift L */
#define	CINTR	0177		/* DEL */

/* flags */
#define LCASE	04
#define ECHO	010
#define CRMOD	020

/* modes */
#define TOPEN	1

/* limits */
#define	TTHIWAT	50
#define	TTLOWAT	30
#define	TTYHOG	100

/* Hardware bits */
#define	DONE	0200
#define	IENABLE	0100

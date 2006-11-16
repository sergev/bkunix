struct fdio {
	int		fd_csrw;	/* Copy of fdc state register */
	int		fd_curtrk;	/* Current track */
	unsigned char	fd_trktab [4];	/* Table of current tracks */
	int		fd_tdown;	/* Head down time (in SOB loops) */
	int		fd_tstep;	/* Track change time */
	unsigned char	fd_trkcor;	/* Write precompensation track number */
	unsigned char	fd_bretry;	/* Max retries */
	unsigned char	fd_flags;	/* Driver data */
	unsigned char	fd_fillb;	/* Fill byte for formatting */
	int		fd_flgptr;	/* Pointer to flag byte */
	unsigned char	fd_flgtab [4];	/* Flag table */
	int		fd_addr;	/* Buffer address in RAM */
	int		fd_wcnt;	/* Number of words to transfer */
	unsigned char	fd_side;	/* Disk side */
	unsigned char	fd_trk;		/* Track */
	unsigned char	fd_unit;	/* Disk unit number */
	unsigned char	fd_sector;	/* Sector */
	unsigned char	fd_wrk1 [18];	/* Driver working area */
	int		fd_maxsec;	/* Number of sectors per track */
	unsigned char	fd_wrk2 [4];	/* Driver working area */
};

struct fdio *ioarea = (struct fdio*) 02000;
int buf [256*10];

void format_track (cyl, head)
{
	register int r4;
	register struct fdio *r3;

	ioarea->fd_trk = cyl;
	ioarea->fd_side = head;
	r3 = ioarea;

	asm ("mov r5,-(sp)");	/* r5 will be corrupted */
	asm ("jsr pc, *$0160012");
	asm ("mov (sp)+,r5");
}

int write_track (track)
{
	register int r4;
	register struct fdio *r3;
	register int *r2;
	int blk;

	/* blk = track * 10 */
	blk = track + track;
	blk = blk + blk + track;
	blk += blk;

	r3 = ioarea;
	r2 = buf;
	r4 = blk;
	asm ("mov $-2560, r1");	/* word cnt, negative for write */
	asm ("mov r4, r0");	/* blk num */
	asm ("mov r5,-(sp)");	/* r5 will be corrupted */
	asm ("jsr pc, *$0160004");
	asm ("bcs 1f");
	asm ("mov (sp)+,r5");
	asm ("clr r0");
	asm ("jmp cret");
	asm ("1: mov (sp)+,r5");
	return -1;
}

int read_track (track)
{
	register int r4;
	register struct fdio *r3;
	register int *r2;
	int blk;

	/* blk = track * 10 */
	blk = track + track;
	blk = blk + blk + track;
	blk += blk;

	r3 = ioarea;
	r2 = buf;
	r4 = blk;
	asm ("mov $2560, r1");	/* word cnt, negative for write */
	asm ("mov r4, r0");	/* blk num */
	asm ("mov r5,-(sp)");	/* r5 will be corrupted */
	asm ("jsr pc, *$0160004");
	asm ("bcs 1f");
	asm ("mov (sp)+,r5");
	asm ("clr r0");
	asm ("jmp cret");
	asm ("1: mov (sp)+,r5");
	return -1;
}

int format (track)
	register int track;
{
	register int i;

	putchar (' ');
	phexdigit (track);
	format_track (track >> 1, track & 1);
	putchar ('.');

	for (i=0; i<256*10; ++i)
		buf[i] = -track;

	if (write_track (track) < 0) {
		printf ("<err");
		printhex (*(unsigned char*) 052);
		printf (">");
		return -1;
	}
	putchar ('.');
	if (read_track (track) < 0) {
		printf ("<err");
		printhex (*(unsigned char*) 052);
		printf (">");
		return -1;
	}
	for (i=0; i<256*10; ++i) {
		if (buf[i] != -track) {
			putchar ('#');
			return -1;
		}
	}
	return 0;
}

int main ()
{
	register int track, retry;
	int save_bretry, c, errors;

	save_bretry = ioarea->fd_bretry;
	ioarea->fd_bretry = 1;		/* Disable retries */
        ioarea->fd_trkcor = 999;	/* Disable write precompensation */
	ioarea->fd_fillb = 0xff;
again:
	/* Stop floppy motor. */
	*(int*) 0177130 = 0;

	printf ("\nInsert another floppy and press Y when ready.\n");
	printf ("Do you really want to format this floppy? (Y/N) ");
	c = getchar();
	putchar (c);
	putchar ('\n');
	if (c == 'n' || c == 'N') {
		ioarea->fd_bretry = save_bretry;
		return 0;
	}
	if (c != 'y' && c != 'Y')
		goto again;

	printf ("\nFormat:");
	errors = 0;
	for (track = 0; track < (81*2); track++) {
		retry = 0;
		while (format (track) < 0) {
			if (++retry >= 3) {
				errors++;
				break;
			}
		}
	}
	printf (errors ? " - FAILED.\n" : " - Done.\n");
	goto again;
}

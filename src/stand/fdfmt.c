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
unsigned buf [10 * 512 / sizeof (short)];

void fd_format (cyl, head)
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

int fd_wrtrack (track, data)
	unsigned *data;
{
	register int r4;
	register struct fdio *r3;
	register unsigned *r2;
	int blk;

	/* blk = track * 10 */
	blk = track + track;
	blk = blk + blk + track;
	blk += blk;

	r3 = ioarea;
	r2 = data;
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

int fd_rdtrack (track, data)
	unsigned *data;
{
	register int r4;
	register struct fdio *r3;
	register unsigned *r2;
	int blk;

	/* blk = track * 10 */
	blk = track + track;
	blk = blk + blk + track;
	blk += blk;

	r3 = ioarea;
	r2 = data;
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

int fd_wrsector (blk, data)
	unsigned *data;
{
	register int r4;
	register struct fdio *r3;
	register unsigned *r2;

	r3 = ioarea;
	r2 = data;
	r4 = blk;
	asm ("mov $-256, r1");	/* word cnt, negative for write */
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

int fd_rdsector (blk, data)
	unsigned *data;
{
	register int r4;
	register struct fdio *r3;
	register unsigned *r2;

	r3 = ioarea;
	r2 = data;
	r4 = blk;
	asm ("mov $256, r1");	/* word cnt, negative for write */
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

int format_track (track)
	register int track;
{
	register int i;

	printf ("\r%3d ", track);
	fd_format (track >> 1, track & 1);

	for (i=0; i<256*10; ++i)
		buf[i] = 0xf6f6;
	if (fd_wrtrack (track, buf) < 0) {
		printf ("format error %#o\n", *(unsigned char*) 052);
		return -1;
	}
	if (fd_rdtrack (track, buf) < 0) {
		printf ("read error %#o\n", *(unsigned char*) 052);
		return -1;
	}
	for (i=0; i<256*10; ++i) {
		if (buf[i] != 0xf6f6) {
			puts ("data error\n");
			return -1;
		}
	}
	return 0;
}

void format ()
{
	register int track, retry;
	int errors, i;

	puts ("\nFormatting floppy, 160 tracks\n");
	errors = 0;
	for (track = 0; track < (80*2); track++) {
		retry = 0;
		while (format_track (track) < 0) {
			if (++retry >= 3) {
				errors++;
				break;
			}
		}
	}
	puts (errors ? " - FAILED.\n" : " - Done.\n");
}

void write_pattern ()
{
	register int s, i;

	puts ("\nWriting test pattern, 1600 sectors\n");
	for (s=0; s<1600; ++s) {
		for (i=0; i<256; ++i)
			buf[i] = s;
		printf ("\r%4d ", s);
		if (fd_wrsector (s, buf) < 0) {
			printf ("write error %#o\n", *(unsigned char*) 052);
		}
	}
	puts ("\n");
}

void test_sector (s)
	register int s;
{
	register int i;

	printf ("\r%4d ", s);
	if (fd_rdsector (s, buf) < 0) {
		printf ("read error %#o\n", *(unsigned char*) 052);
		return;
	}
	for (i=0; i<256; ++i)
		if (buf[i] != s) {
			puts ("data error\n");
			return;
		}
}

void seq_test ()
{
	register int s;

	puts ("\nSequential reading, 1600 sectors\n");
	for (s=0; s<1600; ++s)
		test_sector (s);
	puts ("\n");
}

void zigzag_test ()
{
	register int s;

	puts ("\nZig-zag reading, 0...1599 sectors\n");
	for (s=0; s<1600-35; s+=15) {
		test_sector (s);
		test_sector (s + 35);
	}
	puts ("\n");
}

int main ()
{
	register int save_bretry, c;

	save_bretry = ioarea->fd_bretry;
	ioarea->fd_bretry = 1;		/* Disable retries */
        ioarea->fd_trkcor = 80;		/* Disable write precompensation */
	ioarea->fd_fillb = 0366;
again:
	/* Stop floppy motor. */
	*(int*) 0177130 = 0;

	puts ("\n 1. Formatting floppy");
	puts ("\n 2. Writing test pattern to floppy");
	puts ("\n 3. Sequential reading and checking of all sectors");
	puts ("\n 4. Zig-zag reading and checking of 100 sectors");
	puts ("\n\nCommand: ");
	c = getchar();
	putchar (c);
	putchar ('\n');
	switch (c) {
	case '1':
		format ();
		break;
	case '2':
		write_pattern ();
		break;
	case '3':
		seq_test ();
		break;
	case '4':
		zigzag_test ();
		break;
	}
	goto again;
}

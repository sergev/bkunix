/*
 * Print floppy drive info.
 */
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

int bootdev;
struct fdio ioarea;

void fdinit (io)
	struct fdio *io;
{
	/* When calling floppy BIOS, we must pass an address of
	 * i/o area in R3. Declare two register variables:
	 * the first is always placed in R4 by compiler,
	 * the second - in R3. */
	register char *r4;
	register struct fdio *r3;

	r3 = io;
	((void(*)()) 0160010) ();
}

void fdprint (io)
	register struct fdio *io;
{
	puts ("csrw "); printhex (io->fd_csrw);
	puts (" curtrk "); printhex (io->fd_curtrk);
	puts (" trktab "); printhex (io->fd_trktab[0]);
	puts (" "); printhex (io->fd_trktab[1]);
	puts (" "); printhex (io->fd_trktab[2]);
	puts (" "); printhex (io->fd_trktab[3]);
	puts ("\n");

	puts ("tdown "); printhex (io->fd_tdown);
	puts (" tstep "); printhex (io->fd_tstep);
	puts (" trkcor "); printhex (io->fd_trkcor);
	puts (" bretry "); printhex (io->fd_bretry);
	puts (" flags "); printhex (io->fd_flags);
	puts ("\n");

	puts ("fillb "); printhex (io->fd_fillb);
	puts (" flgptr "); printhex (io->fd_flgptr);
	puts (" flgtab "); printhex (io->fd_flgtab[0]);
	puts (" "); printhex (io->fd_flgtab[1]);
	puts (" "); printhex (io->fd_flgtab[2]);
	puts (" "); printhex (io->fd_flgtab[3]);
	puts (" addr "); printhex (io->fd_addr);
	puts ("\n");

	puts ("wcnt "); printhex (io->fd_wcnt);
	puts (" side "); printhex (io->fd_side);
	puts (" trk "); printhex (io->fd_trk);
	puts (" unit "); printhex (io->fd_unit);
	puts (" sector "); printhex (io->fd_sector);
	puts (" maxsec "); printhex (io->fd_maxsec);
	puts ("\n");
}

int main()
{
	fdinit (&ioarea);
	puts ("After fdinit:\n");
	fdprint (&ioarea);

	puts ("\nSystem area at 2000:\n");
	fdprint ((struct fdio*) 02000);
	return 0;
}

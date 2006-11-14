/*
 * Print floppy drive info.
 */
struct fdio {
	int		fd_csrw;	/* ����� �� ������ �������� ��������� ���� - 0 */
	int		fd_curtrk;	/* ����� ������� ������� */
	unsigned char	fd_trktab [4];	/* ������� ������� ������� */
	int		fd_tdown;	/* ����� ��������� ������� (� ������ SOB) - 20000 */
	int		fd_tstep;	/* ����� �������� � ������� �� ������� */
	unsigned char	fd_trkcor;	/* ����� �������, � ������� ���������� ��������������� ��� ������ - 36 */
	unsigned char	fd_bretry;	/* ����� ��������� �������� ��� ������� - 30 */
	unsigned char	fd_flags;	/* ������� ������ �������� */
	unsigned char	fd_fillb;	/* ��� ���������� �������� ��� �������������� */
	int		fd_flgptr;	/* ��������� �� ���� ��������� */
	unsigned char	fd_flgtab [4];	/* ������� ��������� - 0 */
	int		fd_addr;	/* ����� ������ ��� */
	int		fd_wcnt;	/* ����� ���� ��� ��������� */
	unsigned char	fd_side;	/* ����� ������� ����� */
	unsigned char	fd_trk;		/* ����� ������� */
	unsigned char	fd_unit;	/* ����� ������� */
	unsigned char	fd_sector;	/* ����� ������� */
	unsigned char	fd_wrk1 [22];	/* ������� ������ �������� */
	int		fd_maxsec;	/* ����� �������� �� ������� */
	unsigned char	fd_wrk2 [4];	/* ������� ������ */
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
	printf ("csrw "); printhex (io->fd_csrw);
	printf (" curtrk "); printhex (io->fd_curtrk);
	printf (" trktab "); printhex (io->fd_trktab[0]);
	printf (" "); printhex (io->fd_trktab[1]);
	printf (" "); printhex (io->fd_trktab[2]);
	printf (" "); printhex (io->fd_trktab[3]);
	printf ("\n");

	printf ("tdown "); printhex (io->fd_tdown);
	printf (" tstep "); printhex (io->fd_tstep);
	printf (" trkcor "); printhex (io->fd_trkcor);
	printf (" bretry "); printhex (io->fd_bretry);
	printf (" flags "); printhex (io->fd_flags);
	printf ("\n");

	printf ("fillb "); printhex (io->fd_fillb);
	printf (" flgptr "); printhex (io->fd_flgptr);
	printf (" flgtab "); printhex (io->fd_flgtab[0]);
	printf (" "); printhex (io->fd_flgtab[1]);
	printf (" "); printhex (io->fd_flgtab[2]);
	printf (" "); printhex (io->fd_flgtab[3]);
	printf (" addr "); printhex (io->fd_addr);
	printf ("\n");

	printf ("wcnt "); printhex (io->fd_wcnt);
	printf (" side "); printhex (io->fd_side);
	printf (" trk "); printhex (io->fd_trk);
	printf (" unit "); printhex (io->fd_unit);
	printf (" sector "); printhex (io->fd_sector);
	printf (" maxsec "); printhex (io->fd_maxsec);
	printf ("\n");
}

int main()
{
	fdinit (&ioarea);
	printf ("After fdinit:\n");
	fdprint (&ioarea);

	printf ("\nSystem area at 2000:\n");
	fdprint ((struct fdio*) 02000);
	return 0;
}

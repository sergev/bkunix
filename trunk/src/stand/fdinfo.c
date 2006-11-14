/*
 * Print floppy drive info.
 */
struct fdio {
	int		fd_csrw;	/* Копия по записи регистра состояния НГМД - 0 */
	int		fd_curtrk;	/* Адрес текущей дорожки */
	unsigned char	fd_trktab [4];	/* Таблица текущих дорожек */
	int		fd_tdown;	/* Время опускания головки (в циклах SOB) - 20000 */
	int		fd_tstep;	/* Время перехода с дорожки на дорожку */
	unsigned char	fd_trkcor;	/* Номер дорожки, с которой включается предкомпенсация при записи - 36 */
	unsigned char	fd_bretry;	/* Число повторных операций при ошибках - 30 */
	unsigned char	fd_flags;	/* Рабочая ячейка драйвера */
	unsigned char	fd_fillb;	/* Код заполнения секторов при форматировании */
	int		fd_flgptr;	/* Указатель на байт признаков */
	unsigned char	fd_flgtab [4];	/* Таблица признаков - 0 */
	int		fd_addr;	/* Адрес буфера ОЗУ */
	int		fd_wcnt;	/* Число слов для пересылки */
	unsigned char	fd_side;	/* Номер стороны диска */
	unsigned char	fd_trk;		/* Номер дорожки */
	unsigned char	fd_unit;	/* Номер привода */
	unsigned char	fd_sector;	/* Номер сектора */
	unsigned char	fd_wrk1 [22];	/* Рабочие ячейки драйвера */
	int		fd_maxsec;	/* Число секторов на дорожке */
	unsigned char	fd_wrk2 [4];	/* Рабочие ячейки */
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

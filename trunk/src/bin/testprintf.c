/*
 * Testing vprintf(3).
 */
#include <stdlib.h>

#define fputs(s,f) printstring (s)

static void
printstring (str)
	char *str;
{
	write (1, str, strlen (str));
}

static void
fmtchk (fmt)
	char *fmt;
{
	(void)fputs (fmt, stdout);
	(void)printf (":\t`");
	(void)printf (fmt, 0x12);
	(void)printf ("'\n");
}

static void fmtst1chk (fmt)
	char *fmt;
{
	(void)fputs (fmt, stdout);
	(void)printf (":\t`");
	(void)printf (fmt, 4, 0x12);
	(void)printf ("'\n");
}

static void
fmtst2chk (fmt)
	char *fmt;
{
	(void)fputs (fmt, stdout);
	(void)printf (":\t`");
	(void)printf (fmt, 4, 4, 0x12);
	(void)printf ("'\n");
}

/* This page is covered by the following copyright: */

/* (C) Copyright C E Chew
 *
 * Feel free to copy, use and distribute this software provided:
 *
 *	1. you do not pretend that you wrote it
 *	2. you leave this copyright notice intact.
 */

/*
 * Extracted from exercise.c for glibc-1.05 bug report by Bruce Evans.
 */
#define DEC -123
#define INT 255
#define UNS (~0)

/* Formatted Output Test
 *
 * This exercises the output formatting code.
 */
static void
fp_test ()
{
	int i, j, k, l;
	char buf[7];
	char *prefix = buf;
	char tp[20];

	fputs ("\nFormatted output test\n");
	printf ("prefix  6d      6o      6x      6X      6u\n");
	strcpy (prefix, "%");
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			for (k = 0; k < 2; k++) {
				for (l = 0; l < 2; l++) {
					strcpy (prefix, "%");
					if (i == 0)
						strcat (prefix, "-");
					if (j == 0)
						strcat (prefix, "+");
					if (k == 0)
						strcat (prefix, "#");
					if (l == 0)
						strcat (prefix, "0");
					printf ("%5s |", prefix);
					strcpy (tp, prefix);
					strcat (tp, "6d |");
					printf (tp, DEC);
					strcpy (tp, prefix);
					strcat (tp, "6o |");
					printf (tp, INT);
					strcpy (tp, prefix);
					strcat (tp, "6x |");
					printf (tp, INT);
					strcpy (tp, prefix);
					strcat (tp, "6X |");
					printf (tp, INT);
					strcpy (tp, prefix);
					strcat (tp, "6u |");
					printf (tp, UNS);
					printf ("\n");
				}
			}
		}
	}
	printf ("%10s\n", (char *)0);
	printf ("%-10s\n", (char *)0);
}

int
main ()
{
	static char shortstr[] = "Hi, Z.";
	static char longstr[] = "Good morning, Doctor Chandra.  This is Hal.  \
I am ready for my first lesson today.";

	fmtchk ("%.4x");
	fmtchk ("%04x");
	fmtchk ("%4.4x");
	fmtchk ("%04.4x");
	fmtchk ("%4.3x");
	fmtchk ("%04.3x");

	fmtst1chk ("%.*x");
	fmtst1chk ("%0*x");
	fmtst2chk ("%*.*x");
	fmtst2chk ("%0*.*x");

	printf ("binary format:\t\"%b\"\n", 55, "\20\7b7\6b6\5b5\4b4\3b3\2b2\1b1");
	printf ("nil pointer (padded):\t\"%10p\"\n", (void *)0);

	printf ("decimal negative:\t\"%d\"\n", -2345);
	printf ("octal negative:\t\"%o\"\n", -2345);
	printf ("hex negative:\t\"%x\"\n", -2345);
	printf ("long decimal number:\t\"%ld\"\n", -123456L);
	printf ("long octal negative:\t\"%lo\"\n", -2345L);
	printf ("long unsigned decimal number:\t\"%lu\"\n", -123456L);
	printf ("zero-padded LDN:\t\"%010ld\"\n", -123456L);
	printf ("left-adjusted ZLDN:\t\"%-010ld\"\n", -123456L);
	printf ("space-padded LDN:\t\"%10ld\"\n", -123456L);
	printf ("left-adjusted SLDN:\t\"%-10ld\"\n", -123456L);

	printf ("zero-padded string:\t\"%010s\"\n", shortstr);
	printf ("left-adjusted Z string:\t\"%-010s\"\n", shortstr);
	printf ("space-padded string:\t\"%10s\"\n", shortstr);
	printf ("left-adjusted S string:\t\"%-10s\"\n", shortstr);
	printf ("null string:\t\"%s\"\n", (char *)0);
	printf ("limited string:\t\"%.22s\"\n", longstr);

	printf ("%#03x\n", 1);

	fp_test ();

	return 0;
}

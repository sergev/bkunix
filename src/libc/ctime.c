/*
 * Copyright (c) 1987 Regents of the University of California.
 * This file may be freely redistributed provided that this
 * notice remains attached.
 */
#include <time.h>
#include <string.h>

#define	SECS_PER_MIN	60
#define	MINS_PER_HOUR	60
#define	HOURS_PER_DAY	24
#define	SECS_PER_HOUR	(SECS_PER_MIN * MINS_PER_HOUR)
#define	SECS_PER_DAY	((long) SECS_PER_HOUR * HOURS_PER_DAY)
#define	EPOCH_WDAY	4 /* thursday */

/*
 * Accurate only for the past couple of centuries;
 * that will probably do.
 */
static int isleap(y)
	int y;
{
	if (y % 4 != 0)
		return 0;
	if (y % 100 != 0)
		return 1;
	if (y % 400 == 0)
		return 1;
	return 0;
}

static void
putnumb(cp, n)
	register char *cp;
{
	*cp++ = (n / 10) % 10 + '0';
	*cp++ = n % 10 + '0';
}

/*
** A la X3J11
*/
char *
asctime(timeptr)
	register struct tm *timeptr;
{
	static char	wday_name[7][3] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char	mon_name[12][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char	result[26];

	strcpy(result, "Day Mon 00 00:00:00 1900\n");
	memcpy(result, wday_name[timeptr->tm_wday], 3);
	memcpy(result+4, mon_name[timeptr->tm_mon], 3);
	putnumb(result+8, timeptr->tm_mday);
	putnumb(result+11, timeptr->tm_hour);
	putnumb(result+14, timeptr->tm_min);
	putnumb(result+17, timeptr->tm_sec);
	putnumb(result+20, 19 + timeptr->tm_year / 100);
	putnumb(result+22, timeptr->tm_year);
	return result;
}

struct state {
	int		timecnt;
	int		typecnt;
	int		charcnt;
	long		gmtoff;		/* GMT offset in seconds */
	int		isdst;		/* used to set tm_isdst */
	char		chars[8];
};

static struct state	s;

static int		tz_is_set;

char *			tzname[2] = {
	"GMT",
	"GMT"
};

long			timezone = 0;
int			daylight = 0;

void
tzset()
{
	tz_is_set = 1;

	/* GMT is default */
	s.timecnt = 0;
	s.gmtoff = 0;
	(void) strcpy(s.chars, "GMT");
	tzname[0] = tzname[1] = s.chars;
	timezone = 0;
	daylight = 0;
}

static int	mon_lengths[2][12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int	year_lengths[2] = {
	365, 366
};

static struct tm *
offtime(clock, offset)
	long *clock;
	long offset;
{
	register struct tm *	tmp;
	register long		days;
	register long		rem;
	register int		y;
	register int		yleap;
	register int *		ip;
	static struct tm	tm;

	tmp = &tm;
	days = *clock / SECS_PER_DAY;
	rem = *clock % SECS_PER_DAY;
	rem += offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECS_PER_HOUR);
	rem = rem % SECS_PER_HOUR;
	tmp->tm_min = (int) (rem / SECS_PER_MIN);
	tmp->tm_sec = (int) (rem % SECS_PER_MIN);
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % 7);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += 7;
	y = 1970;
	if (days >= 0)
		for ( ; ; ) {
			yleap = isleap(y);
			if (days < (long) year_lengths[yleap])
				break;
			++y;
			days = days - (long) year_lengths[yleap];
		}
	else do {
		--y;
		yleap = isleap(y);
		days = days + (long) year_lengths[yleap];
	} while (days < 0);
	tmp->tm_year = y - 1900;
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days = days - (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
	tmp->tm_zone = "";
	tmp->tm_gmtoff = offset;
	return tmp;
}

struct tm *
localtime(timep)
	long *timep;
{
	register struct tm *		tmp;
	register int			i;
	long				t;

	if (! tz_is_set)
		(void) tzset();
	t = *timep;
	/*
	** To get (wrong) behavior that's compatible with System V Release 2.0
	** you'd replace the statement below with
	**	tmp = offtime((long) (t + s.gmtoff), 0L);
	*/
	tmp = offtime(&t, s.gmtoff);
	tmp->tm_isdst = s.isdst;
	tzname[tmp->tm_isdst] = &s.chars[0];
	tmp->tm_zone = &s.chars[0];
	return tmp;
}

struct tm *
gmtime(clock)
	long *clock;
{
	register struct tm *	tmp;

	tmp = offtime(clock, 0L);
	tzname[0] = "GMT";
	tmp->tm_zone = "GMT";		/* UCT ? */
	return tmp;
}

char *
ctime(t)
	long *t;
{
	return(asctime(localtime(t)));
}

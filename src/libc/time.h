/*
 * Copyright (c) 1983, 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#ifndef _TIME_H_
#define _TIME_H_ 1

#include <ansidecl.h>

/*
 * Structure returned by gmtime and localtime calls (see ctime(3)).
 */
struct tm {
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
	long	tm_gmtoff;
	char	*tm_zone;
};

extern char *tzname[2];
extern long timezone;
extern int daylight;

struct tm *gmtime PARAMS((long*));
struct tm *localtime PARAMS((long*));
char *asctime PARAMS((struct tm*));
char *ctime PARAMS((long*));

#endif /* _TIME_H_ */

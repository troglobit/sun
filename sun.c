/*

Simple SUNRISET front-end application

(c) Joachim Nilsson, 2017

Released to the public domain by Joachim Nilsson, December 2017

 */
#include "config.h"

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "tzalias.h"
#include "sunriset.h"

#define TIMEZONE "/etc/timezone"
#define ZONETAB  "/usr/share/zoneinfo/zone.tab"

/* From The Practice of Programming, by Kernighan and Pike */
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))
#define PRINTF(fmt, args...) if (verbose > 0) printf(fmt, ##args)

static time_t     now;
static struct tm *tm;
static struct tm  tm_specific;
static time_t     offset = 0;
static int        utc = 0;
static int        verbose = 1;
static int        do_wait = 0;
static int        show_seconds = 0;
extern char      *__progname;

static time_t timediff(void)
{
	static int done = 0;
	static time_t diff;

	if (done)
		return diff;

//	tzset();
//	diff = -timezone;
	diff = tm->tm_gmtoff;
	done = 1;

	return diff;
}

/*
 * Converts the optional `-w TIME` offset to a relative offset.
 * Supports e.g.. 30m, -15m, 13s, 1h ... or just 3600
 */
static time_t convert_offset(char *arg)
{
	int mult = 1;
	long long val = 0;

	if (!arg)
		return 0;

	if (strpbrk(arg, "hH"))
		mult = 3600;
	if (strpbrk(arg, "mM"))
		mult = 60;

	sscanf(arg, "%lld", &val);
	val *= mult;

	/* MAX offset == +/- 6h, otherwise your location is wrong */
	if (val < -6 * 3600)
		val = -6 * 3600;
	if (val >  6 * 3600)
		val = 6 * 3600;

	return val;
}

static void convert(double ut, int *h, int *m, int *s)
{
	int s_local;

	/* Add the seconds offset first. */
	ut += timediff() / 3600;

	*h = (int)floor(ut);
	ut -= *h;

	*m = (int)(ut * 60);
	ut -= (double)*m / 60;

	s_local = (int)(ut * 3600 + 0.5);

	if (s == NULL) {
		/* Round to the nearest minute. */
		if (s_local >= 30) {
			++*m;
			if (*m == 60) {
				++*h;
				*m = 0;
				/* Not sure how to handle *h == 24. */
			}
		}
	}
	else
		*s = s_local;
}

static char *lctime_r(double ut, char *buf, size_t len)
{
	int h, m, s;

	if (show_seconds) {
		convert(ut, &h, &m, &s);
		snprintf(buf, len, "%02d:%02d:%02d", h, m, s);
	}
	else {
		convert(ut, &h, &m, NULL);
		snprintf(buf, len, "%02d:%02d", h, m);
	}

	return buf;
}

static char *lctime(double ut)
{
	static char buf[10];

	return lctime_r(ut, buf, sizeof(buf));
}

static int riset(int mode, double lat, double lon, int year, int month, int day)
{
	double rise, set;
//	char bufr[10], bufs[10];

	sun_rise_set(year, month, day, lon, lat, &rise, &set);

	if (mode)
		PRINTF("Sun rises %s", lctime(rise));
	if (!mode)
		PRINTF("Sun sets %s", lctime(set));
	if (mode == -1)
		PRINTF(", sets %s", lctime(set));
	PRINTF(" %s\n", tm->tm_zone);
//	printf("Sun rises %s, sets %s %s\n", lctime_r(rise, bufr, sizeof(bufr)),
//	       lctime_r(set, bufs, sizeof(bufs)), tm->tm_zone);

	if (do_wait > 0) {
		int h, m, s;
		time_t then, sec;

		if (mode)
			convert(rise, &h, &m, NULL);
		else
			convert(set, &h, &m, NULL);

		/* Adjust for sunset/sunrise regardless of timezone */
		h = h - tm->tm_hour;
		if (h < 0)
			h += 24;
		m = m - tm->tm_min;
		if (m < 0)
			m += 60;
		then = now + 3600 * h + 60 * m;
		sec = then - now + offset;

		/* Pretty printing */
		h = sec / 60 / 60;
		m = sec / 60 - h * 60;
		s = sec - m * 60 - h * 60 * 60;
		PRINTF("Sleeping %dh%dm%ds ...\n", h, m, s);
		sleep(sec);
	}

	return 0;
}

static int sunrise(double lat, double lon, int year, int month, int day)
{
	return riset(1, lat, lon, year, month, day);
}

static int sunset(double lat, double lon, int year, int month, int day)
{
	return riset(0, lat, lon, year, month, day);
}

static int all(double lat, double lon, int year, int month, int day)
{
	double daylen, civlen, nautlen, astrlen;
	double rise, set, civ_start, civ_end, naut_start, naut_end;
	double astr_start, astr_end;
	int rs, civ, naut, astr;
	char bufr[10], bufs[10];

	daylen = day_length(year, month, day, lon, lat);
	civlen = day_civil_twilight_length(year, month, day, lon, lat);
	nautlen = day_nautical_twilight_length(year, month, day, lon, lat);
	astrlen = day_astronomical_twilight_length(year, month, day, lon, lat);

	PRINTF("Day length:                 %5.2f hours\n", daylen);
	PRINTF("With civil twilight         %5.2f hours\n", civlen);
	PRINTF("With nautical twilight      %5.2f hours\n", nautlen);
	PRINTF("With astronomical twilight  %5.2f hours\n", astrlen);
	PRINTF("Length of twilight: civil   %5.2f hours\n", (civlen - daylen) / 2.0);
	PRINTF("                  nautical  %5.2f hours\n", (nautlen - daylen) / 2.0);
	PRINTF("              astronomical  %5.2f hours\n", (astrlen - daylen) / 2.0);

	rs = sun_rise_set(year, month, day, lon, lat, &rise, &set);
	civ = civil_twilight(year, month, day, lon, lat, &civ_start, &civ_end);
	naut = nautical_twilight(year, month, day, lon, lat, &naut_start, &naut_end);
	astr = astronomical_twilight(year, month, day, lon, lat, &astr_start, &astr_end);

	PRINTF("Sun at south %s %s\n", lctime((rise + set) / 2.0), tm->tm_zone);

	switch (rs) {
	case 0:
		printf("Sun rises %s, sets %s %s\n",
		       lctime_r(rise, bufr, sizeof(bufr)),
		       lctime_r(set, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		PRINTF("Sun above horizon\n");
		break;

	case -1:
		PRINTF("Sun below horizon\n");
		break;
	}

	switch (civ) {
	case 0:
		printf("Civil twilight starts %s, ends %s %s\n",
		       lctime_r(civ_start, bufr, sizeof(bufr)),
		       lctime_r(civ_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		PRINTF("Never darker than civil twilight\n");
		break;

	case -1:
		PRINTF("Never as bright as civil twilight\n");
		break;
	}

	switch (naut) {
	case 0:
		printf("Nautical twilight starts %s, ends %s %s\n",
		       lctime_r(naut_start, bufr, sizeof(bufr)),
		       lctime_r(naut_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		PRINTF("Never darker than nautical twilight\n");
		break;

	case -1:
		PRINTF("Never as bright as nautical twilight\n");
		break;
	}

	switch (astr) {
	case 0:
		printf("Astronomical twilight starts %s, ends %s %s\n",
		       lctime_r(astr_start, bufr, sizeof(bufr)),
		       lctime_r(astr_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		PRINTF("Never darker than astronomical twilight\n");
		break;

	case -1:
		PRINTF("Never as bright as astronomical twilight\n");
		break;
	}

	return 0;
}

static void chomp(char *str)
{
	size_t len;

	if (!str)
		return;

	len = strlen(str) - 1;
	while (len && str[len] == '\n')
		str[len--] = 0;
}

static int alias(char *tz, size_t len)
{
	size_t i;

	for (i = 0; i < NELEMS(tzalias); i++) {
		if (strcmp(tzalias[i].name, tz))
			continue;

		strncpy(tz, tzalias[i].alias, len);
		return 1;
	}

	return 0;
}

static int probe(double *lat, double *lon)
{
	int found = 0, again = 0;
	FILE *fp;
	char *ptr, tz[43], buf[80];

	ptr = getenv("TZ");
	if (!ptr) {
		fp = fopen(TIMEZONE, "r");
		if (!fp)
			return 0;

		if (fgets(tz, sizeof(tz), fp))
			chomp(tz);
		fclose(fp);
	} else {
		strncpy(tz, ptr, sizeof(tz));
		tz[sizeof(tz) - 1] = 0;
	}

retry:
//	printf("tz: '%s'\n", tz);
	fp = fopen(ZONETAB, "r");
	if (!fp)
		return 0;

	while ((fgets(buf, sizeof(buf), fp))) {
		ptr = strstr(buf, tz);
		if (!ptr)
			continue;
		*ptr = 0;
		ptr = buf;
		while (*ptr != ' ' && *ptr != '	')
			ptr++;

		if (sscanf(ptr, "%lf%lf", lat, lon) == 2) {
			*lat /= 100.0;
			*lon /= 100.0;
			found = 1;
		}
//		printf("buf: '%s', lat: %lf, lon: %lf\n", ptr, *lat, *lon);
		break;
	}
	fclose(fp);

	if (!found) {
//		printf("Cannot find your coordinates using time zone: %s\n", tz);
		if (!again) {
			again++;
			/* Use "sizeof - 1" to preserve the last byte */
			/* for NUL and to avoid a compiler warning.   */
			if (alias(tz, sizeof(tz) - 1))
				goto retry;
		}
//		puts("");
	}

	return found;
}

static int interactive(double *lat, double *lon, int *year, int *month, int *day)
{
	char buf[80];

	printf("Latitude (+ is north) and longitude (+ is east) : ");
	if (fgets(buf, sizeof(buf), stdin))
		sscanf(buf, "%lf %lf", lat, lon);

	printf("Input date ( yyyy mm dd ) (ctrl-C exits): ");
	if (fgets(buf, 80, stdin))
		sscanf(buf, "%d %d %d", year, month, day);

	return 1;
}

static int usage(int code)
{
	printf("Usage:\n"
	       "  %s [-ahilrsuvw] [-d YYYY-MM-DD] [-o OFFSET] [-x FLAG] [-- +/-latitude +/-longitude]\n"
	       "\n"
	       "Options:\n"
	       "  -a      Show all relevant times and exit\n"
	       "  -h      This help text\n"
	       "  -i      Interactive mode\n"
	       "  -l      Increased verbosity, enable log messages\n"
	       "  -r      Sunrise mode\n"
	       "  -s      Sunset mode\n"
	       "  -u      Use UTC everywhere, not local time\n"
	       "  -v      Show program version and exit\n"
	       "  -w      Wait until sunset or sunrise\n"
	       "  -d ARG  Specific date, in \"yyyy-mm-dd\" format.\n"
	       "  -o ARG  Time offset to adjust wait, e.g. -o -30m\n"
	       "          maximum allowed offset: +/- 6h\n"
	       "  -x ARG  Extra options:\n"
	       "          -x secs  Show times with seconds.\n"
	       "\n"
	       "Bug report address: %s\n",
	       __progname, PACKAGE_BUGREPORT);

	return code;
}

int main(int argc, char *argv[])
{
	int c, op = 0, ok = 0;
	int year, month, day;
	double lon = 0.0, lat;
	int spec_ymd = 0;
	int spec_year, spec_mon, spec_mday;

	while ((c = getopt(argc, argv, "ad:hilo:rsuvwx:")) != EOF) {
		switch (c) {
		case 'h':
			return usage(0);

		case 'i':
			verbose++;
			ok = interactive(&lat, &lon, &year, &month, &day);
			break;

		case 'l':
			verbose++;
			break;

		case 'a':
			verbose++;
			do_wait--;
			/* fallthrough */
		case 'r':
		case 's':
			op = c;
			break;

		case 'u':
			utc = 1;
			break;

		case 'v':
			puts(PACKAGE_VERSION);
			return 0;

		case 'd':
			if (sscanf(optarg, "%d-%d-%d", &year, &month, &day) == 3)
			   spec_ymd = 1;
			else {
			   fprintf(stderr, "error: invalid date specification\n");
			   return usage(1);
			}
			break;

		case 'o':
			offset = convert_offset(optarg);
			break;

		case 'w':
			verbose--;
			do_wait++;
			break;

		case 'x':
			/*
			 * This option was created to allow new options for which other
			 * letters were already taken.  For example, "-s" would have been
			 * good for "show times with seconds", but "-s" was already taken.
			 */
			if (strcmp(optarg, "secs") == 0)
				show_seconds = 1;
			/* Add new "strcmp()"s as needed. */
			else {
			   fprintf(stderr, "error: unsupported flag '%s'\n", optarg);
			   return usage(1);
			}
			break;

		case ':':	/* missing param for option */
		case '?':	/* unknown option */
		default:
			return usage(1);
		}
	}

	if (spec_ymd) {
		/*
		 * Create Epoch Time for the specified date.  "3:00
		 * a.m." is used to account for ST/DST changes,
		 * which usually take effect by that time of day.
		 */
		spec_mday = day;
		spec_mon  = month - 1;
		spec_year = year - 1900;

		tm_specific.tm_sec    = 0;
		tm_specific.tm_min    = 0;
		tm_specific.tm_hour   = 3;
		tm_specific.tm_mday   = spec_mday;
		tm_specific.tm_mon    = spec_mon;
		tm_specific.tm_year   = spec_year;
		tm_specific.tm_gmtoff = 0;

		now = mktime(&tm_specific);

		/*
		 * If the returned day of month, month, or year
		 * changed, the specified date wasn't valid.
		 */
		if ( (tm_specific.tm_mday != spec_mday) ||
		     (tm_specific.tm_mon  != spec_mon)  ||
		     (tm_specific.tm_year != spec_year) ) {
			fprintf(stderr, "error: '%4d-%02d-%02d' is not a valid date\n",
					  year, month, day);
			return 1;
		}
	} else {
		now = time(NULL);
	}

	if (utc)
		tm = gmtime(&now);
	else
		tm = localtime(&now);

	if (!ok) {
		if (optind < argc)
			lat = atof(argv[optind++]);
		if (optind < argc)
			lon = atof(argv[optind]);

		year = 1900 + tm->tm_year;
		month = 1 + tm->tm_mon;
		day = tm->tm_mday;

//		PRINTF("latitude %f longitude %f date %d-%02d-%02d %d:%d (%s)\n",
//		       lat, lon, year, month, day, tm->tm_hour, tm->tm_min, tm->tm_zone);
		if (lon != 0.0)
			ok = 1;

		if (!ok)
			ok = probe(&lat, &lon);
	} else {
		tm->tm_year = year  - 1900;
		tm->tm_mon  = month - 1;
		tm->tm_mday = day;
	}

	if (!ok)
		return usage(1);

	switch (op) {
	case 'a':
		return all(lat, lon, year, month, day);

	case 'r':
		return sunrise(lat, lon, year, month, day);

	case 's':
		return sunset(lat, lon, year, month, day);

	default:
		verbose++;
		break;
	}

	return riset(-1, lat, lon, year, month, day);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */

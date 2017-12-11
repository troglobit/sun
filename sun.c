#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "sunriset.h"

#define TIMEZONE "/etc/timezone"
#define ZONETAB  "/usr/share/zoneinfo/zone.tab"

#define PRINTF(fmt, args...) if (verbose > 0) printf(fmt, ##args)

static time_t now;
static struct tm *tm;

static int  verbose = 1;
static int  do_wait = 0;
extern char *__progname;

static void convert(double ut, int *h, int *m)
{
	*h = (int)floor(ut);
	*m = (int)(60 * (ut - floor(ut)));
}

static char *ut2str(double ut)
{
	int h, m;
	static char buf[10];

	convert(ut, &h, &m);
	snprintf(buf, sizeof(buf), "%02d:%02d", h, m);

	return buf;
}

static void print(const char *fmt, double up, double dn)
{
	int uh, um, dh, dm;

	convert(up, &uh, &um);
	convert(dn, &dh, &dm);

	PRINTF(fmt, uh, um, dh, dm);
}

static int riset(int mode, double lat, double lon, int year, int month, int day)
{
	double rise, set;

	sun_rise_set(year, month, day, lon, lat, &rise, &set);
	if (mode)
		PRINTF("Sun rises %s", ut2str(rise));
	if (!mode)
		PRINTF("Sun sets %s", ut2str(set));
	if (mode == -1)
		PRINTF(", sets %s", ut2str(set));
	PRINTF(" UTC\n");
//	print("Sun rises %02d:%02d, sets %02d:%02d UTC\n", rise, set);

	if (do_wait > 0) {
		int h, m;
		time_t then, sec;

		if (mode)
			convert(rise, &h, &m);
		else
			convert(set, &h, &m);

		tm->tm_hour = h;
		tm->tm_min  = m;
		then = mktime(tm);

		if (then < now)
			then += 60 * 60 * 24;

		sec = then - now;
		PRINTF("Sleeping %ld sec ...\n", sec);
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

	PRINTF("Sun at south %s UTC\n", ut2str((rise + set) / 2.0));

	switch (rs) {
	case 0:
		print("Sun rises %02d:%02d, sets %02d:%02d UTC\n", rise, set);
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
		print("Civil twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      civ_start, civ_end);
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
		print("Nautical twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      naut_start, naut_end);
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
		print("Astronomical twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      astr_start, astr_end);
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

static int probe(double *lat, double *lon)
{
	int found = 0;
	FILE *fp;
	char *ptr, tz[42], buf[80];

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
	printf("Usage: %s [-hip] [+/-latitude +/-longitude]\n"
	       "\n"
	       "Options:\n"
	       "  -a  Show all relevant times and exit\n"
	       "  -h  This help text\n"
	       "  -i  Interactive mode\n"
	       "  -r  Sunrise mode\n"
	       "  -s  Sunset mode\n"
	       "  -w  Wait until sunset or sunrise\n"
	       "\n", __progname);

	return code;
}

int main(int argc, char *argv[])
{
	int c, op = 0, ok = 0;
	int year, month, day;
	double lon = 0.0, lat;

	while ((c = getopt(argc, argv, "ahirsvw")) != EOF) {
		switch (c) {
		case 'h':
			return usage(0);

		case 'i':
			verbose++;
			ok = interactive(&lat, &lon, &year, &month, &day);
			break;

		case 'a':
			verbose++;
			do_wait--;
			/* fallthrough */
		case 'r':
		case 's':
			op = c;
			break;

		case 'v':
			verbose++;
			break;

		case 'w':
			verbose--;
			do_wait++;
			break;

		case ':':	/* missing param for option */
		case '?':	/* unknown option */
		default:
			return usage(1);
		}
	}

	now = time(NULL);
	tm = gmtime(&now);

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

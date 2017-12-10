#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sunriset.h"

static char *ut2str(double ut)
{
	int h, m;
	static char buf[10];

	h = (int)floor(ut);
	m = (int)(60 * (ut - floor(ut)));

	snprintf(buf, sizeof(buf), "%02d:%02d", h, m);

	return buf;
}

static void print(const char *fmt, double up, double dn)
{
	int uh, um, dh, dm;

	uh = (int)floor(up);
	um = (int)(60 * (up - floor(up)));
	dh = (int)floor(dn);
	dm = (int)(60 * (dn - floor(dn)));

	printf(fmt, uh, um, dh, dm);
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

	printf("Day length:                 %5.2f hours\n", daylen);
	printf("With civil twilight         %5.2f hours\n", civlen);
	printf("With nautical twilight      %5.2f hours\n", nautlen);
	printf("With astronomical twilight  %5.2f hours\n", astrlen);
	printf("Length of twilight: civil   %5.2f hours\n", (civlen - daylen) / 2.0);
	printf("                  nautical  %5.2f hours\n", (nautlen - daylen) / 2.0);
	printf("              astronomical  %5.2f hours\n", (astrlen - daylen) / 2.0);

	rs = sun_rise_set(year, month, day, lon, lat, &rise, &set);
	civ = civil_twilight(year, month, day, lon, lat, &civ_start, &civ_end);
	naut = nautical_twilight(year, month, day, lon, lat, &naut_start, &naut_end);
	astr = astronomical_twilight(year, month, day, lon, lat, &astr_start, &astr_end);

	printf("Sun at south %s UTC\n", ut2str((rise + set) / 2.0));

	switch (rs) {
	case 0:
		print("Sun rises %02d:%02d, sets %02d:%02d UTC\n", rise, set);
		break;

	case +1:
		printf("Sun above horizon\n");
		break;

	case -1:
		printf("Sun below horizon\n");
		break;
	}

	switch (civ) {
	case 0:
		print("Civil twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      civ_start, civ_end);
		break;

	case +1:
		printf("Never darker than civil twilight\n");
		break;

	case -1:
		printf("Never as bright as civil twilight\n");
		break;
	}

	switch (naut) {
	case 0:
		print("Nautical twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      naut_start, naut_end);
		break;

	case +1:
		printf("Never darker than nautical twilight\n");
		break;

	case -1:
		printf("Never as bright as nautical twilight\n");
		break;
	}

	switch (astr) {
	case 0:
		print("Astronomical twilight starts %02d:%02d, ends %02d:%02d UTC\n",
		      astr_start, astr_end);
		break;

	case +1:
		printf("Never darker than astronomical twilight\n");
		break;

	case -1:
		printf("Never as bright as astronomical twilight\n");
		break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int year, month, day;
	double lon, lat;

	if (argc < 3) {
		char buf[80];

		printf("Latitude (+ is north) and longitude (+ is east) : ");
		fgets(buf, 80, stdin);
		sscanf(buf, "%lf %lf", &lat, &lon);

		printf("Input date ( yyyy mm dd ) (ctrl-C exits): ");
		fgets(buf, 80, stdin);
		sscanf(buf, "%d %d %d", &year, &month, &day);
	} else {
		time_t now;
		struct tm *tm;

		lat = atof(argv[1]);
		lon = atof(argv[2]);

		now = time(NULL);
		tm = localtime(&now);
		year = 1900 + tm->tm_year;
		month = 1 + tm->tm_mon;
		day = tm->tm_mday;

		printf("latitude %f longitude %f date %d-%02d-%02d %d:%d (%s)\n",
		       lat, lon, year, month, day, tm->tm_hour, tm->tm_min, tm->tm_zone);
	}

	return all(lat, lon, year, month, day);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */

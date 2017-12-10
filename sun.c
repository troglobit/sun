#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sunriset.h"

int main(int argc, char *argv[])
{
	int year, month, day;
	double lon, lat;
	double daylen, civlen, nautlen, astrlen;
	double rise, set, civ_start, civ_end, naut_start, naut_end;
	double astr_start, astr_end;
	int rs, civ, naut, astr;
	char buf[80];

	if (argc < 3) {
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
	printf("Times are in UT, approximately == UTC\n");

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

	printf("Sun at south %5.2fh UT\n", (rise + set) / 2.0);

	switch (rs) {
	case 0:
		printf("Sun rises %5.2fh UT, sets %5.2fh UT\n", rise, set);
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
		printf("Civil twilight starts %5.2fh, " "ends %5.2fh UT\n", civ_start, civ_end);
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
		printf("Nautical twilight starts %5.2fh, " "ends %5.2fh UT\n", naut_start, naut_end);
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
		printf("Astronomical twilight starts %5.2fh, " "ends %5.2fh UT\n", astr_start, astr_end);
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

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */

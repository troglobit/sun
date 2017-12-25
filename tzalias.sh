#!/bin/sh
#
# Public Domain script to extra TZ aliases, by Joachim Nilsson
#
# Creates tzalias.h from the latest tzdata archive, which is maintained
# by IANA and can be found at https://www.iana.org/time-zones
#
# Download and unpack the archive into a separate directory, then run
# this script from that directory.  The output is the file tzalias.h
#
FILES="africa australasia backward backzone europe etcetera northamerica southamerica"
HEADER=tzalias.h

echo "/* This is a generated file, see tzalias.sh to update it */" >$HEADER
echo ""                  >>$HEADER
echo "struct tz_alias {" >>$HEADER
echo "	char *name;"     >>$HEADER
echo "	char *alias;"    >>$HEADER
echo "} tzalias[] = {"   >>$HEADER

cat $FILES | awk '
/^Link\s*([^\s]*)\s([^\s]*)/ {				\
	printf "\t{ \"%s\", \"%s\" },\n", $3, $2;	\
} ' | sort               >>$HEADER

#echo "	{ NULL, NULL }"  >>$HEADER
echo "};"                >>$HEADER

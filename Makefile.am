doc_DATA                = README.md LICENSE
EXTRA_DIST              = $(doc_DATA)

if ENABLE_LIBRARY
pkgconfigdir            = $(libdir)/pkgconfig
pkgconfig_DATA          = libsunriset.pc

library_includedir      = $(includedir)
library_include_HEADERS = sunriset.h

lib_LTLIBRARIES         = libsunriset.la
libsunriset_la_SOURCES  = sunriset.c sunriset.h
libsunriset_la_CPPFLAGS = -DSUNRISET_LIB
libsunriset_la_CFLAGS   = -std=gnu99
libsunriset_la_CFLAGS  += -W -Wall -Wextra -Wundef -Wunused -Wstrict-prototypes
libsunriset_la_CFLAGS  += -Werror-implicit-function-declaration -Wshadow -Wcast-qual
libsunriset_la_LDFLAGS  = $(AM_LDFLAGS) -version-info 0:0:0
endif

bin_PROGRAMS            = sun
sun_SOURCES             = sun.c
sun_CFLAGS              = -W -Wall -Wextra
sun_CPPFLAGS            = -D_GNU_SOURCE
sun_LDADD               = -lm
if ENABLE_LIBRARY
sun_LDFLAGS             = -static
sun_LDADD              += libsunriset.la
else
sun_CPPFLAGS           += -DSUNRISET_LIB
sun_SOURCES            += sunriset.c sunriset.h
endif

## Generate MD5 checksum file
MD5 = md5sum
md5-dist:
	@for file in $(DIST_ARCHIVES); do	\
		$(MD5) $$file > $$file.md5;	\
	done

## Check if tagged in git
release-hook:
	if [ ! `git tag | grep v$(PACKAGE_VERSION)` ]; then				\
		echo;									\
		printf "\e[1m\e[41mCannot find release tag v$(PACKAGE_VERSION)\e[0m\n";	\
		printf "\e[1m\e[5mDo release anyway?\e[0m "; read yorn;			\
		if [ "$$yorn" != "y" -a "$$yorn" != "Y" ]; then				\
			printf "OK, aborting release.\n";				\
			exit 1;								\
		fi;									\
		echo;									\
	else										\
		echo;									\
		printf "\e[1m\e[42mFound GIT release tag $(PACKAGE_VERSION)\e[0m\n";	\
		printf "\e[1m\e[44m>>Remember to push tags!\e[0m\n";			\
		echo;									\
	fi

## Target to run when building a release
release: distcheck release-hook md5-dist
	@for file in $(DIST_ARCHIVES); do						\
		printf "$$file    \tDistribution tarball\n";				\
		printf "$$file.md5\t"; cat $$file.md5 | cut -f1 -d' ';			\
		mv $$file* ../;								\
	done

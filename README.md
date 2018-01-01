About
-----

Simple public domain library and application that shows sunset and
sunrise based on your latitude,longitude.  The application can also
be used with cron to run a script, see section Goal below.

The library, which can be used entirely stand-alone, is based on the
excellent code by [Paul Schlyter][].


Example
-------

Without any arguments the `sun` goes dumpster diving in system files
`/etc/timezone` and `/usr/share/zoneinfo/zone.tab` to figure out
your latitude and longitude.

On my system, in mid December here in Sweden, the result is:

```sh
$ cat /etc/timezone
Europe/Stockholm
$ sun
Sun rises 07:31, sets 13:49 UTC
```

You can set the environment variable `TZ` to check other locations:

```sh
$ TZ="Africa/Luanda" sun
Sun rises 04:42, sets 17:18 UTC
```

For more precision, you can of course provide your exact latitude and
longitude.  See the built-in usage text for more help: `sun -h`


Cron
----

To launch applications at sunrise/sunset using cron, simply:

```sh
$ crontab -e
01 00 * * * sun -r -w -o -30m; play english.au
```

This plays Linus Torvalds' classic audio file 30 minutes before sunrise.

**NOTE:** You may want to set the `$PATH` in your crontab, or use an
  absolute path to your programs, otherwise cron will not find them.


Goal
----

The goal of this project is to provide just enough intelligence to
my home automation system so it can control the indoor lights, which
are operated using 433 MHz outlet switches.


Building
--------

The `sun` is built with the GNU configure and build system.  Simply run
the following commands to build and install:

```sh
$ ./configure
$ make
$ sudo make install
```

The `sunriset.c` code can be built as a library, use `--enable-library`
with the configure script to enable this optional feature.

If you built from GIT, or have modified any of the `.ac` or `.am` files,
you have to run the following to (re-)create the configure script:

```sh
.$ /autogen.sh
```

For that to work you need to have `autoconf`, `automake`, and `libtool`
installed.  These tools are not needed when building from an official
released tarball.



[Paul Schlyter]: http://stjarnhimlen.se/

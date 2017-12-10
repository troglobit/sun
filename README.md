About
-----

Simple public domain library and application that shows sunset and
sunrise based on your latitude,longitude.

The library, which can be used entirely stand-alone, is based on the
excellent code by [Paul Schlyter][].


Example
-------

Without any arguments the `sun` goes dumpster diving in the system files
`/etc/timezone` and `/usr/share/zoneinfo/zone.tab` to figure out the
latitude and longitude.

On my system, in mid December here in Sweden, the result is:

```
$ cat /etc/timezone
Europe/Stockholm
$ sun
Sun rises 07:31, sets 13:49 UTC
$ TZ="Africa/Luanda" sun
Sun rises 04:42, sets 17:18 UTC
```

See the built-in usage text for more help: `sun -h`


Goal
----

The goal of this project is to provide just enough intelligence to
my home automation system so it can control the indoor lights, which
are operated using 433 MHz outlet switches.

[Paul Schlyter]: http://stjarnhimlen.se/

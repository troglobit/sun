AC_INIT(sun, 1.0, https://github.com/troglobit/sun/issues)
AM_INIT_AUTOMAKE([1.11 foreign])

AC_CONFIG_SRCDIR([sunriset.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile libsunriset.pc])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
LT_INIT

# Optional features
AC_ARG_ENABLE(library,
        AS_HELP_STRING([--enable-library], [Build sunriset library]),,[enable_library=no])

AM_CONDITIONAL(ENABLE_LIBRARY, [test "x$enable_library" = "xyes"])

# Generate all files
AC_OUTPUT

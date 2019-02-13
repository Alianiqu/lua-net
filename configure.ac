#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lua-net], [0.0.1], [no@whe.re])
AC_CONFIG_SRCDIR([src/core.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB


# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([socket strerror])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT

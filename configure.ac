#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lua-net], [0.0.1], [no@whe.re])
AC_CONFIG_SRCDIR([src/core.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability subdir-objects])


# libtool
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

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

# Lua module and scripts
AC_ARG_WITH([lua-pc],
	[AS_HELP_STRING(
		[--with-lua-pc=PKG],
		[Specify pkg-config package name for lua]
	)], [], [with_lua_pc=no])

if test "x$with_lua_pc" != "xno"; then
	# exit with error if not found
	PKG_CHECK_MODULES([LUA], [$with_lua_pc], [LUAPKGCONFIG=$with_lua_pc])
fi

if "x$with_lua_pc" != "xno"; then
	PKG_CHECK_MODULES([LUA], [$with_lua_pc],
		[LUAPKGCONFIG=$with_lua_pc
		enable_lua=yes],
		[enable_lua=no])
fi

PKG_CHECK_MODULES([LUA], [lua], [LUAPKGCONFIG=lua],
	[PKG_CHECK_MODULES([LUA], [lua5.2], [LUAPKGCONFIG=lua5.2],
		[PKG_CHECK_MODULES([LUA], [lua5.1], [LUAPKGCONFIG=lua5.1],
			[AS_IF([test "x$enable_lua" = "xyes"],
				[AC_MSG_ERROR([Lua not found. Please use --with-lua-pc=PKG])],
				[enable_lua=no])]
		)]
	)])

AC_MSG_CHECKING([Lua version])
PKG_CHECK_VAR([LUA_VERSION], [$LUAPKGCONFIG], [V],,
	[PKG_CHECK_VAR([LUA_VERSION], [$LUAPKGCONFIG], [major_version])])
AC_MSG_RESULT([$LUA_VERSION])
AC_SUBST([LUA_LIBDIR], [$libdir/lua/$LUA_VERSION])
AC_SUBST([LUA_SHAREDIR], [$datadir/lua/$LUA_VERSION])


AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT

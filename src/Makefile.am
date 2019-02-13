luasharedir=$(LUA_SHAREDIR)
lualibdir=$(LUA_LIBDIR)
luadir=$(luasharedir)
sodir=$(lualibdir)/net

lua_DATA=net.lua

lib_LTLIBRARIES = libcore.la
libcore_la_SOURCES = core.c

AM_LDFLAGS = -Wl,-E

AM_CFLAGS=-I$(top_builddir)/src -I$(top_srcdir)/src $(LUA_CFLAGS) -DVERSION=\"$(VERSION)\"

libcore_la_CFLAGS = -fPIC -DPIC $(AM_CFLAGS)

libcore_la_LDFLAGS = \
	-shared \
	-L$(top_builddir)/ \
	-Wl,-soname,core.so.$(firstword $(subst ., ,$(VERSION)))

# libcore_la_LIBADD =

install-exec-local: install-libLTLIBRARIES
	mkdir -p $(DESTDIR)$(lualibdir)/net/
	mv $(DESTDIR)$(libdir)/libcore.so.0.0.0 $(DESTDIR)$(lualibdir)/net/core.so
	rm $(DESTDIR)$(libdir)/libcore.*

uninstall-local:
	$(RM) $(DESTDIR)$(lualibdir)/lxc/core.so*

net.lua:

EXTRA_DIST= \
	net.lua

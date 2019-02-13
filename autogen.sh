#!/bin/sh
#

set -x

touch NEWS
touch README
touch AUTHORS

test -d autom4te.cache && rm -rf autom4te.cache
libtoolize || exit 1
aclocal || exit 1
autoheader || exit 1
autoconf || exit 1
automake --add-missing --copy || exit 1

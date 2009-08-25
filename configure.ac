# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This file is part of paprefs.
#
# Copyright 2006-2008 Lennart Poettering
#
# paprefs is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# paprefs is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with paprefs. If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.57)
AC_INIT([paprefs],[0.9.9],[mzcncersf (at) 0pointer (dot) de])
AC_CONFIG_SRCDIR([src/paprefs.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign 1.9 -Wall])

AC_SUBST(PACKAGE_URL, [http://0pointer.de/lennart/projects/paprefs/])

if type -p stow > /dev/null && test -d /usr/local/stow ; then
   AC_MSG_NOTICE([*** Found /usr/local/stow: default install prefix set to /usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION} ***])
   ac_default_prefix="/usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION}"
fi

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_TYPE_SIGNAL
AC_HEADER_STDC

PKG_CHECK_MODULES(GUILIBS, [ gtkmm-2.4 libglademm-2.4 sigc++-2.0 gconfmm-2.6 dbus-glib-1])
AC_SUBST(GUILIBS_CFLAGS)
AC_SUBST(GUILIBS_LIBS)

PKG_CHECK_MODULES(LIBPULSE, libpulse)
LIBPULSE_MODLIBEXECDIR=`pkg-config libpulse --variable modlibexecdir`
AC_SUBST(LIBPULSE_MODLIBEXECDIR)

# If using GCC specifiy some additional parameters
if test "x$GCC" = "xyes" ; then
   CFLAGS="$CFLAGS -pipe -Wall -W -Wno-unused-parameter"
   CXXFLAGS="$CXXFLAGS -pipe -Wall -W"
fi

# LYNX documentation generation
AC_ARG_ENABLE(lynx,
        AS_HELP_STRING(--disable-lynx,Turn off lynx usage for documentation generation),
[case "${enableval}" in
  yes) lynx=yes ;;
  no)  lynx=no ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --disable-lynx) ;;
esac],[lynx=yes])

if test x$lynx = xyes ; then
   AC_CHECK_PROG(have_lynx, lynx, yes, no)

   if test x$have_lynx = xno ; then
     AC_MSG_ERROR([*** Sorry, you have to install lynx or use --disable-lynx ***])
   fi
fi

AM_CONDITIONAL([USE_LYNX], [test "x$lynx" = xyes])

AM_GNU_GETTEXT([external])

IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=paprefs
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([
po/Makefile.in
Makefile
src/Makefile
doc/Makefile
doc/README.html])

AC_OUTPUT

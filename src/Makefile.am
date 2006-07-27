# $Id$
#
# This file is part of paprefs.
#
# paprefs is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# paprefs is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with paprefs; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

desktopdir = $(datadir)/applications

bin_PROGRAMS=paprefs

dist_pkgdata_DATA=paprefs.glade
dist_desktop_DATA=paprefs.desktop

paprefs_SOURCES=paprefs.cc

paprefs_LDADD=$(AM_LDADD) $(GUILIBS_LIBS)
paprefs_CXXFLAGS=$(AM_CXXFLAGS) $(GUILIBS_CFLAGS)
paprefs_CXXFLAGS+=-DGLADE_FILE=\"$(pkgdatadir)/paprefs.glade\"

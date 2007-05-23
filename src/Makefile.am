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
gladedir = $(pkgdatadir)
localedir = $(datadir)/locale

bin_PROGRAMS=paprefs
glade_DATA=paprefs.glade
desktop_in_files=paprefs.desktop.in 
desktop_DATA=$(desktop_in_files:.desktop.in=.desktop) 

paprefs_SOURCES=paprefs.cc

paprefs_LDADD=$(AM_LDADD) $(GUILIBS_LIBS)
paprefs_CXXFLAGS=$(AM_CXXFLAGS) $(GUILIBS_CFLAGS)
paprefs_CXXFLAGS+=-DGLADE_FILE=\"$(gladedir)/paprefs.glade\" -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST = $(glade_DATA) $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

@INTLTOOL_DESKTOP_RULE@

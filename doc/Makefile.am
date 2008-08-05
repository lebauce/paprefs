# This file is part of paprefs.
#
# Copyright 2006-2008 Lennart Poettering
#
# paprefs is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 2.1 of the
# License, or (at your option) any later version.
#
# paprefs is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with paprefs. If not, see
# <http://www.gnu.org/licenses/>.

noinst_DATA = README.html README
EXTRA_DIST = $(noinst_DATA) style.css README.html.in

MAINTAINERCLEANFILES = README README.html
CLEANFILES =

if USE_LYNX
README: README.html
	lynx --dump $^ | sed 's,file://localhost/.*/doc/README.html,README,' > $@

CLEANFILES += README
endif

tidy: README.html
	tidy -e < README.html

.PHONY: tidy


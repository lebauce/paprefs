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

EXTRA_DIST= config.rpath m4/ChangeLog \
	bootstrap.sh \
	README \
	LICENSE \
	intltool-merge.in \
	intltool-update.in \
	intltool-extract.in

SUBDIRS=src doc po

MAINTAINERCLEANFILES=README
noinst_DATA = README

README:
	rm -f README
	$(MAKE) -C doc README
	cd $(srcdir) && ln -s doc/README README

homepage: all dist
	test -d $$HOME/homepage/private
	mkdir -p $$HOME/homepage/private/projects/paprefs
	cp paprefs-@PACKAGE_VERSION@.tar.gz $$HOME/homepage/private/projects/paprefs
	cp doc/README.html doc/screenshot.png doc/style.css $$HOME/homepage/private/projects/paprefs
	ln -sf README.html $$HOME/homepage/private/projects/paprefs/index.html

DISTCLEANFILES = \
        intltool-extract \
        intltool-merge \
        intltool-update 

fedora-snapshot: dist
	cp $(distdir).tar.gz $$HOME/cvs.fedora/paprefs/devel/$(distdir).svn`date +%Y%m%d`.tar.gz

.PHONY: homepage

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	if test -d .git ; then \
		git pull ; \
		chmod u+w ${distdir}/ChangeLog || true ; \
		git-changelog.perl > ${distdir}/ChangeLog ; \
	fi

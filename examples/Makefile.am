SUBDIRS = src data

EXTRA_DIST = README CHANGES LICENSE NOTICE acsite.m4 mkinstalldir install-sh

dist-hook:
	tar cf - doc \
             src/ssg/tux/data \
             src/ssg/majik/data \
             src/fnt/data     \
             data     \
	   | ( cd $(distdir) ; tar xf - )


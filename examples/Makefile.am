SUBDIRS = examples

EXTRA_DIST = README CHANGES LICENSE NOTICE acsite.m4

dist-hook:
	tar cf - ../doc \
             src/ssg/tux/data \
             src/ssg/majik/data \
             src/fnt/data     \
	   | ( cd $(distdir) ; tar xf - )


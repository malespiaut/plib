SUBDIRS = src

EXTRA_DIST = README CHANGES LICENSE NOTICE acsite.m4

dist-hook:
	tar cf - doc \
	   | ( cd $(distdir) ; tar xf - )


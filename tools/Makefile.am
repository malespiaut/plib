SUBDIRS = src

EXTRA_DIST = autogen.sh README NOTICE tools.dsw

dist-hook:
	tar cf - doc \
	   | ( cd $(distdir) ; tar xf - )


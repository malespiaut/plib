SUBDIRS = src

EXTRA_DIST = autogen.sh README CHANGES NOTICE plib_examples.dsp \
	plib_examples.dsw

dist-hook:
	tar cf - doc \
             src/ssg/tux/data \
             src/ssg/majik/data \
             src/fnt/data     \
             data     \
	   | ( cd $(distdir) ; tar xf - )


if BUILD_FNT

noinst_PROGRAMS = fnt_test slideshow hello_world

hello_world_SOURCES = hello_world.cxx

fnt_test_SOURCES = fnt_test.cxx

slideshow_SOURCES = slideshow.cxx

hello_world_LDADD = -lplibpw -lplibfnt -lplibsg -lplibul $(OGL_LIBS)
fnt_test_LDADD    = -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)
slideshow_LDADD   = -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)

endif

EXTRA_DIST = fnt_test.dsp slideshow.dsp


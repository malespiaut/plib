if BUILD_SSG

noinst_PROGRAMS = majik_demo

majik_demo_SOURCES = elevation_map.h \
                     image_map.h     \
                     majik_demo.cxx

majik_demo_LDADD = -lplibjs -lplibssg -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)

endif

EXTRA_DIST = majik_demo.dsp


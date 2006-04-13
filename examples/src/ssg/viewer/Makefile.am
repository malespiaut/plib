if BUILD_SSG

noinst_PROGRAMS = viewer pview

viewer_SOURCES = viewer.cxx

pview_SOURCES = pview.cxx

viewer_LDADD = -lplibssg -lplibpuaux -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) \
	$(OGL_LIBS)

pview_LDADD = -lplibssg -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)

endif

EXTRA_DIST = viewer.dsp


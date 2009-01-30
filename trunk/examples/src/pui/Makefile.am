if BUILD_PUI

noinst_PROGRAMS = simple complex widget_list PointPicker widget_test

simple_SOURCES  = simple.cxx

complex_SOURCES = complex.cxx

widget_test_SOURCES = widget_test.cxx

widget_list_SOURCES = widget_list.cxx

PointPicker_SOURCES = PointPicker.cxx

simple_LDADD = -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)

complex_LDADD = -lplibpuaux -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) $(OGL_LIBS)

widget_test_LDADD = -lplibpuaux -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) \
	$(OGL_LIBS)

widget_list_LDADD = -lplibpuaux -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) \
	$(OGL_LIBS)

PointPicker_LDADD = -lplibpuaux -lplibpu -lplibfnt -lplibsg -lplibul $(GLUT_LIBS) \
	$(OGL_LIBS)

endif

EXTRA_DIST = complex.dsp simple.dsp PointPicker.dsp widget_list.dsp widget_test.dsp


if BUILD_PW

noinst_PROGRAMS = pw_demo pw_pui lorenz

pw_demo_SOURCES = pw_demo.cxx
pw_pui_SOURCES  = pw_pui.cxx
lorenz_SOURCES  = lorenz.cxx

pw_demo_LDADD = -lplibpw -lplibul $(OGL_LIBS)
pw_pui_LDADD  = -lplibpw -lplibpu -lplibfnt -lplibsg -lplibul $(OGL_LIBS)
lorenz_LDADD  = -lplibpw -lplibpu -lplibfnt -lplibsg -lplibul $(OGL_LIBS)

endif

EXTRA_DIST = pw_demo.dsp lorenz.dsp


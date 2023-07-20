if BUILD_SG

lib_LIBRARIES = libplibsg.a

include_HEADERS = sg.h

libplibsg_a_SOURCES = sg.cxx sgd.cxx \
                      sgIsect.cxx sgdIsect.cxx \
                      sgPerlinNoise.cxx

AM_CPPFLAGS = -I$(top_srcdir)/src/util

endif

EXTRA_DIST = sg.dsp



if BUILD_PSL

lib_LIBRARIES = libplibpsl.a

include_HEADERS = psl.h pslPrivate.h

libplibpsl_a_SOURCES = psl.cxx pslCodeGen.cxx pslRun.cxx \
                       pslStatement.cxx pslSymbols.cxx pslToken.cxx \
                       pslExpression.cxx pslProgram.cxx

INCLUDES = -I$(top_srcdir)/src/sg -I$(top_srcdir)/src/util

endif

EXTRA_DIST = psl.dsp


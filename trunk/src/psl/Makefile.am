
if BUILD_PSL

lib_LIBRARIES = libplibpsl.a

include_HEADERS = psl.h pslContext.h pslLocal.h \
                  pslCompiler.h pslSymbol.h

libplibpsl_a_SOURCES = psl.cxx pslCodeGen.cxx pslContext.cxx \
                       pslCompiler.cxx pslSymbols.cxx pslToken.cxx \
                       pslExpression.cxx pslProgram.cxx pslDump.cxx

INCLUDES = -I$(top_srcdir)/src/sg -I$(top_srcdir)/src/util

endif

EXTRA_DIST = psl.dsp


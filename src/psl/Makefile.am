
if BUILD_PSL

lib_LIBRARIES = libplibpsl.a

include_HEADERS = psl.h

libplibpsl_a_SOURCES = psl.cxx pslCodeGen.cxx pslCompiler.cxx pslCompiler.h \
                       pslContext.cxx pslContext.h pslDump.cxx pslError.cxx \
                       pslExpression.cxx pslLocal.h pslOpcodes.h \
                       pslProgram.cxx pslSymbol.h pslSymbols.cxx \
                       pslToken.cxx

INCLUDES = -I$(top_srcdir)/src/util

endif

EXTRA_DIST = psl.dsp


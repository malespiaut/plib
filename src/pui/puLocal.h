
#include "pu.h"

#ifndef WIN32
#  ifndef macintosh
#    include <GL/glx.h>
#  else
#    include <agl.h>
#  endif
#endif

/*
  glGetCurrentContext()
*/

#ifdef WIN32
#  define glGetCurrentContext() wglGetCurrentContext()
#else
#  if defined(macintosh)
#     define glGetCurrentContext() aglGetCurrentContext()
#  else
#     define glGetCurrentContext() glXGetCurrentContext()
#  endif
#endif

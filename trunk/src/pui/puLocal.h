
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  ifndef macintosh
#    include <GL/glx.h>
#  endif
#endif

#include <limits.h>
#include <math.h>

#ifndef PU_NOT_USING_GLUT
#  ifdef FREEGLUT_IS_PRESENT
#    include <GL/freeglut.h>
#  else
#    ifdef GLUT_IS_PRESENT
#      include <GL/glut.h>
#    else
        /* No GLUT?!? */
#    endif
#  endif
#endif
#include "pu.h"


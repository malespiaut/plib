#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <GL/glut.h>
#include <plib/sg.h>
#include <plib/pu.h>
#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <plib/ul.h>

#include "event.h"
#include "bones.h"

void createJoint ( sgVec4 colour, Bone *b ) ;


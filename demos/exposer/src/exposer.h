
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
#include "model.h"
#include "bones.h"
#include "boneGUI.h"
#include "vertices.h"
#include "timebox.h"
#include "floor.h"
#include "load_save.h"

extern EventList * eventList ;
extern TimeBox   *   timebox ;
extern Floor     *    ground ;
extern ssgRoot   * skinScene ;
extern ssgRoot   * boneScene ;
extern ssgRoot   *sceneScene ;


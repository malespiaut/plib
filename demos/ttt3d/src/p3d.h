
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

#ifdef WIN32
#include <windows.h>
#endif

#include <plib/ssg.h>

#include <GL/glut.h>
#include <plib/sl.h>
#include <plib/js.h>
#include <plib/fnt.h>

#ifndef M_PI
#  define M_PI 3.14159265
#endif

#include "game.h"

class GFX ;
class GUI ;
class SoundSystem ;
class Puzzle ;
class Cell ;
class Level ;

extern GFX         *gfx        ;
extern GUI         *gui        ;
extern SoundSystem *sound      ;
extern Puzzle      *puzzle     ;
extern Level        level      ;

extern int      game_state ;

extern ssgRoot *scene            ;
//extern const char *ttt3d_datadir ;

void startLevel () ;
void makeMove   () ;
void ttt3dMainLoop () ;
void initMaterials () ;

#include "level.h"
#include "gfx.h"
#include "gui.h"
#include "material.h"
#include "status.h"
#include "sound.h"
#include "cell.h"
#include "puzzle.h"

void spinUp    () ;
void spinDown  () ;
void spinLeft  () ;
void spinRight () ;
void zoomIn    () ;
void zoomOut   () ;


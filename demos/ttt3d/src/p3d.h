/*
     This file is part of TTT3D - Steve's 3D TicTacToe Player.
     Copyright (C) 2001  Steve Baker

     TTT3D is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     TTT3D is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with TTT3D; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


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


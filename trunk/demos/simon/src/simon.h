/*
     S.I.M.O.N - Simple Interface for Making Oliver's programs Nice
     Copyright (C) 2002  Steve Baker

     This library is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <plib/ssg.h>

#define SI_MAX_MODELS 500

void siUpdate () ;
void siRun () ;
int  siLoad ( const char *filename ) ;
void siPosition ( int hh, float x, float y,
                          float z = 0.0f,
                          float h = 0.0f,
                          float p = 0.0f,
                          float r = 0.0f ) ;
 
void siSpeedAndDirection ( int hh, float s, float h, float p = 0.0f ) ;
 
void siVelocity ( int hh, float x, float y,
                          float z = 0.0f,
                          float h = 0.0f,
                          float p = 0.0f,
                          float r = 0.0f ) ;

float siGetPositionX ( int h ) ;
float siGetPositionY ( int h ) ;
float siGetPositionZ ( int h ) ;
float siGetPositionH ( int h ) ;
float siGetPositionP ( int h ) ;
float siGetPositionR ( int h ) ;

float siJoystickLR () ;
float siJoystickUD () ;

bool  siJoystickA () ;
bool  siJoystickB () ;
bool  siJoystickC () ;
bool  siJoystickD () ;
bool  siJoystickL () ;
bool  siJoystickR () ;


/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
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
#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <plib/pw.h>

#define MAX_SHAPE  5
static ssgRoot      *scene    = NULL ;
static ssgTransform *shape [ MAX_SHAPE ] = { NULL } ;

static sgVec4 colour [ MAX_SHAPE ] =
{
  { 1, 0, 0, 1 },
  { 0, 1, 0, 1 },
  { 0, 0, 1, 1 },
  { 1, 1, 0, 1 },
  { 0, 1, 1, 1 }
} ;

/*
  Something to make some interesting motion
*/

static void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;

  sgSetCoord   ( & campos, 0.0f, -6.0f, 0.0f, 0.0f, 0.0f, 0.0f ) ;
  ssgSetCamera ( & campos ) ;

  for ( int i = 0 ; i < MAX_SHAPE ; i++ )
  {
    sgCoord pos ;
    sgSetCoord ( & pos, 2.0f*(float)(i % 3)-2.0f, 0.0f,
                        2.0f*(float)(i / 3)-1.5f,
                        frameno/100.0f, frameno/20.0f, 0.0f ) ;
    shape [ i ] -> setTransform ( & pos ) ;
  }
}


static void redraw ()
{
  update_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  ssgCullAndDraw ( scene ) ;

  pwSwapBuffers () ;
}



static void init_graphics ()
{
  pwInit ( 100, 100, 640, 480, false, "PLIB Shapes Demo", true, 0 ) ;
  pwSetCallbacks ( NULL, NULL, NULL, NULL, NULL ) ;

  /*
    Initialise SSG
  */

  ssgInit () ;

  /*
    Some basic OpenGL setup
  */

  glClearColor ( 0.2f, 0.7f, 1.0f, 1.0f ) ;
  glEnable ( GL_DEPTH_TEST ) ;

  /*
    Set up the viewing parameters
  */

  ssgSetFOV     ( 60.0f, 0.0f ) ;
  ssgSetNearFar ( 1.0f, 700.0f ) ;

  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
}


/*
  Create a simple database
*/

static void load_database ()
{
  scene    = new ssgRoot      ;

  ssgSimpleState *state = new ssgSimpleState () ;
  state -> setShininess ( 8.0f ) ;

  for ( int i = 0 ; i < MAX_SHAPE ; i++ )
  {
    ssgaShape *s ;

    shape [ i ] = new ssgTransform ;
    scene -> addKid ( shape [ i ] ) ;

    switch ( i )
    {
      case 0 : s = new ssgaCube     ( 1000 ) ; break ;
      case 1 : s = new ssgaCylinder ( 1000 ) ; break ;
      case 2 : s = new ssgaSphere   ( 1000 ) ;
               ((ssgaSphere*)s) -> setLatLongStyle ( FALSE) ; break ;
      case 3 : s = new ssgaSphere   ( 1000 ) ;
               ((ssgaSphere*)s) -> setLatLongStyle ( TRUE ) ; break ;
      case 4 :
      default: s = new ssgaTeapot   ( 1000 ) ; break ; 
    }

    s -> setColour ( colour[i] ) ;
    s -> setKidState ( state ) ;
    shape [ i ] -> addKid ( s ) ;
  }
}



/*
  The works.
*/

int main ( int, char ** )
{
  init_graphics () ;
  load_database () ;

  while ( 1 )
    redraw () ;

  return 0 ;
}




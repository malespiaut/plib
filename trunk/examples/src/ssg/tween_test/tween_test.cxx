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
#include <GL/gl.h>
#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <plib/ul.h>
#include <plib/pw.h>

static ssgRoot            *scene      = NULL ;
static ssgTweenController *tween_ctrl = NULL ;


/*
  Something to make some interesting motion
*/

static void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;

  /*
    Make the camera pan sinusoidally left and right
    morphing the sphere/spike as we go.
  */

  tween_ctrl -> selectBank ( fabs(sin(frameno/100.0)) ) ;

  sgSetCoord ( & campos, 0.0f, -5.0f, 0.0f, 25.0 * sin(frameno/100.0), 0.0f, 0.0f ) ;
  ssgSetCamera ( & campos ) ;
}


/*
  The GLUT redraw event
*/

static void redraw ()
{
  glClearColor ( 0.5f, 0.1f, 0.1f, 1.0 ) ;
  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  update_motion () ;

  ssgCullAndDraw ( scene ) ;
}


static void init_graphics ()
{
  pwInit ( 100, 100, 640, 480, false, "PLIB Window Demo", true, 0 ) ;
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

  sgVec4 black = { 0,0,0,1 } ;
  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.0f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, black ) ;
}


/*
  Load a simple database
*/

static void load_database ()
{
  /*
    Create a root node with a Tween controller beneath it.
  */

  scene      = new ssgRoot            ;
  tween_ctrl = new ssgTweenController ;
  scene -> addKid ( tween_ctrl ) ;

  /*
    Construct a sphere (which we'll use as a template
    and then discard
  */

  ssgaSphere *sp = new ssgaSphere ( 100 ) ;

  /*
    For each ssgVtxTable within the sphere, generate
    an ssgTween node
  */

  for ( int i = 0 ; i < sp -> getNumKids () ; i++ )
  {
    ssgVtxTable *vt = (ssgVtxTable *)( sp -> getKid ( i ) ) ;

    assert ( vt -> isAKindOf ( ssgTypeVtxTable () ) ) ;

    /*
      Make two new sets of vertex array.
    */

    ssgVertexArray   *v0 = new ssgVertexArray   ;
    ssgNormalArray   *n0 = new ssgNormalArray   ;
    ssgTexCoordArray *t0 = new ssgTexCoordArray ;
    ssgColourArray   *c0 = new ssgColourArray   ;

    ssgVertexArray   *v1 = new ssgVertexArray   ;
    ssgNormalArray   *n1 = new ssgNormalArray   ;
    ssgTexCoordArray *t1 = new ssgTexCoordArray ;
    ssgColourArray   *c1 = new ssgColourArray   ;

    /*
      For every vertex in the sphere's VtxTable...
    */

    for ( int i = 0 ; i < vt -> getNumVertices () ; i++ )
    {
      /*
        Copy the sphere's vertex (unmolested) into
        the vertex arrays that are destined to become
        bank zero.
      */

      v0 -> add ( vt -> getVertex   (i) ) ;
      n0 -> add ( vt -> getNormal   (i) ) ;
      t0 -> add ( vt -> getTexCoord (i) ) ;
      c0 -> add ( vt -> getColour   (i) ) ;

      /*
        For bank one, make some vertices half
        the usual size - and others twice the
        usual size.  This is a spikey ball.
      */

      sgVec3 vx ;
      sgVec4 co ;

      sgCopyVec3 ( vx, vt -> getVertex (i) ) ;
      sgScaleVec3 ( vx, (i&7) ? 0.5 : 2.0 ) ;

      /*
        Put random colours on the vertices.
      */

      sgSetVec4 ( co, (float)(rand()&0xFF)/255.0f,
                      (float)(rand()&0xFF)/255.0f,
                      (float)(rand()&0xFF)/255.0f, 1.0f ) ;

      /*
        Cheat and make the normals and texture coords be
        the same as the sphere.
      */

      v1 -> add ( vx ) ;
      n1 -> add ( vt -> getNormal   (i) ) ;
      t1 -> add ( vt -> getTexCoord (i) ) ;
      c1 -> add ( co ) ;
    }

    /*
      For each VtxTable in the sphere, create a
      Tween node that morphs between the sphere
      and the spikey sphere.
    */

    ssgTween *tw = new ssgTween ( GL_TRIANGLE_STRIP ) ;
    tw -> newBank ( v0, n0, t0, c0 ) ;
    tw -> newBank ( v1, n1, t1, c1 ) ;

    /*
      Add it into the tween controller
      (although there could be a lot of hierarchy
      between the two if you wanted that)
    */

    tween_ctrl -> addKid ( tw ) ;
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
  {
    ulMilliSecondSleep ( 16 ) ;
    redraw () ;
    pwSwapBuffers () ;
  }

  return 0 ;
}


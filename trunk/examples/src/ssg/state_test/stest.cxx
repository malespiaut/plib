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

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

static ssgRoot      *scene  = NULL ;
static ssgTransform *object = NULL ;
static ssgStateSelector *big_red_switch = NULL ;

/*
  Something to make some interesting motion
  for both Object and the camera.
*/

static void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;
  sgCoord objpos ;

  /*
    Spin Object, make the camera pan sinusoidally left and right
  */

  sgSetCoord ( & campos, 0.0f, -5.0f, 1.0f,
                          /*25.0 * sin(frameno/100.0)*/0.0f, 0.0f, 0.0f ) ;
  sgSetCoord ( & objpos, 0.0f,  0.0f, 0.0f, frameno, 0.0f, 0.0f ) ;

  ssgSetCamera ( & campos ) ;
  object -> setTransform ( & objpos ) ;

  if ( frameno % 600 == 0 )
    big_red_switch -> selectStep ( 0 ) ;

  if ( frameno % 600 == 300 )
    big_red_switch -> selectStep ( 1 ) ;
}



/*
  The GLUT window reshape event
*/

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}



/*
  The GLUT keyboard event
*/

static void keyboard ( unsigned char, int, int )
{
  exit ( 0 ) ;
}



/*
  The GLUT redraw event
*/

static void redraw ()
{
  update_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  ssgCullAndDraw ( scene ) ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}



static void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "Viewer" ;
  fake_argv[1] = "Simple Scene Graph : Viewer Program." ;
  fake_argv[2] = NULL ;

  /*
    Initialise GLUT
  */

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 640, 480 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( redraw   ) ;
  glutReshapeFunc        ( reshape  ) ;
  glutKeyboardFunc       ( keyboard ) ;
 
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



static ssgEntity *make_herring ()
{
  big_red_switch = new ssgStateSelector ( 2 ) ;

  ssgSimpleState *st0 = new ssgSimpleState ;
  ssgSimpleState *st1 = new ssgSimpleState ;

  /* Texture ... or not */

  st0 -> setTexture ( "herring.inta" ) ;
  st0 -> enable  ( GL_TEXTURE_2D ) ;

  st1 -> disable ( GL_TEXTURE_2D ) ;

  /* Other Stuff */

  st0 -> setShadeModel  ( GL_SMOOTH     ) ;
  st1 -> setShadeModel  ( GL_SMOOTH     ) ;

  st0 -> enable         ( GL_CULL_FACE  ) ;
  st1 -> enable         ( GL_CULL_FACE  ) ;

  st0 -> setTranslucent () ;
  st1 -> setTranslucent () ;

  st0 -> enable         ( GL_ALPHA_TEST ) ;
  st1 -> enable         ( GL_ALPHA_TEST ) ;

  st0 -> setAlphaClamp  ( 0.9f           ) ;
  st1 -> setAlphaClamp  ( 0.9f           ) ;

  st0 -> enable         ( GL_BLEND      ) ;
  st1 -> enable         ( GL_BLEND      ) ;

  /* Lighting (where the magic happens) */

  /* A RED herring (because the polygon is red). */
  st0 -> enable            ( GL_LIGHTING ) ;
  st0 -> enable            ( GL_COLOR_MATERIAL ) ;
  st0 -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  st0 -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  st0 -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  st0 -> setShininess      ( 0 ) ;

  /* Hopefully, a GREEN rectangle. */
  st1 -> enable            ( GL_LIGHTING ) ;
  st1 -> disable           ( GL_COLOR_MATERIAL ) ;
  st1 -> setMaterial       ( GL_AMBIENT , 0, 1, 0, 1 ) ;
  st1 -> setMaterial       ( GL_DIFFUSE , 0, 1, 0, 1 ) ;
  st1 -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  st1 -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  st1 -> setShininess      ( 0 ) ;

  /* Set up the selector */

  big_red_switch -> setStep ( 0, st0 ) ;
  big_red_switch -> setStep ( 1, st1 ) ;

  big_red_switch -> selectStep ( 0 ) ; /* Textured! */

  sgVec4 *scolors = new sgVec4 [ 1 ] ;
  sgVec3 *scoords = new sgVec3 [ 6 ] ;
  sgVec2 *tcoords = new sgVec2 [ 6 ] ;
  sgVec3 *snorms  = new sgVec3 [ 1 ] ;

  /* RED! */
  sgSetVec4 ( scolors [ 0 ], 1.0f, 0.0f, 0.0f, 1.0f ) ;
  sgSetVec3 ( snorms[0],  0.0f,  1.0f,  0.0f ) ;

  sgSetVec3(scoords[0], -1.0, 0.0, 0.0 ) ;
  sgSetVec3(scoords[1],  1.0, 0.0, 0.0 ) ;
  sgSetVec3(scoords[2], -1.0, 0.0, 1.0 ) ;
  sgSetVec3(scoords[3],  1.0, 0.0, 1.0 ) ;
  sgSetVec3(scoords[4], -1.0, 0.0, 0.0 ) ;
  sgSetVec3(scoords[5],  1.0, 0.0, 0.0 ) ;

  sgSetVec2(tcoords[0], 0.0, 0.0 ) ;
  sgSetVec2(tcoords[1], 1.0, 0.0 ) ;
  sgSetVec2(tcoords[2], 0.0, 1.0 ) ;
  sgSetVec2(tcoords[3], 1.0, 1.0 ) ;
  sgSetVec2(tcoords[4], 0.0, 0.0 ) ;
  sgSetVec2(tcoords[5], 1.0, 0.0 ) ;

  ssgLeaf *gset = new ssgVTable ( GL_TRIANGLE_STRIP,
                     6, scoords,
                     1, snorms,
                     6, tcoords,
                     1, scolors ) ;

  gset -> setState ( big_red_switch ) ;

  return gset ;
}


/*
  Load a simple database
*/

static void load_database ()
{
  /*
    Set up the path to the data files
  */

  ssgModelPath   ( "." ) ;
  ssgTexturePath ( "." ) ;

  /*
    Create a root node - and a transform to position
    the object beneath that (in the tree that is).
  */

  scene  = new ssgRoot      ;
  object = new ssgTransform ;

  /*
    Load the models - optimise them a bit
    and then add them into the scene.
  */

  ssgEntity *obj_obj = make_herring () ;

  object -> addKid ( obj_obj ) ;

  //ssgFlatten       ( obj_obj ) ;
  //ssgStripify      ( object  ) ;
  scene -> addKid ( object ) ;
  scene -> print () ;
}



/*
  The works.
*/

int main ( int, char ** )
{
  init_graphics () ;
  load_database () ;
  glutMainLoop  () ;
  return 0 ;
}




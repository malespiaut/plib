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


#include  "p3d.h"

static unsigned int lastGLUTKeystroke = 0 ;

static void getGLUTSpecialKeystroke ( int key, int, int )
{
  lastGLUTKeystroke = 256 + key ;
}

static void getGLUTKeystroke ( unsigned char key, int, int )
{
  lastGLUTKeystroke = key ;
}

int getGLUTKeystroke ()
{
  int k = lastGLUTKeystroke ;
  lastGLUTKeystroke = 0 ;
  return k ;
}

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}

static void initWindow ( int w, int h )
{
  int fake_argc = 1 ;
  char *fake_argv[3] ;

  fake_argv[0] = "TTT3D" ;
  fake_argv[1] = "TTT3D by Steve Baker." ;
  fake_argv[2] = NULL ;

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( w, h ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( ttt3dMainLoop ) ;
  glutKeyboardFunc       ( getGLUTKeystroke ) ;
  glutSpecialFunc        ( getGLUTSpecialKeystroke ) ;
  glutReshapeFunc        ( reshape ) ;
#ifndef WIN32
  glutIdleFunc           ( glutPostRedisplay ) ;
#endif
}


GFX::GFX ()
{
  initWindow ( 640, 480 ) ;

  ssgInit  () ;
 
  static int firsttime = 1 ;

  if ( firsttime )
  {
    firsttime = 0 ;
    initMaterials () ;
  }

  ssgSetFOV ( 75.0f, 0.0f ) ;
  ssgSetNearFar ( 0.01f, 10.0f ) ;

  sgCoord cam ;
  sgSetVec3 ( cam.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( cam.hpr, 0, 0, 0 ) ;
  ssgSetCamera ( & cam ) ;
}


void GFX::update ()
{
  sgVec3 sunposn   ;
  sgVec4 skycol ;
  sgVec4 white = { 1.0, 1.0, 1.0, 1.0 } ;

  sgSetVec3 ( sunposn, -0.2f, -0.5f, 0.2f ) ;

  sgSetVec4 ( skycol, 0.4f, 0.4f, 0.8f, 1.0f ) ;

  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , white ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , white ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, white ) ;

  /* Clear the screen */

  glClearColor ( skycol[0], skycol[1], skycol[2], skycol[3] ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  glEnable ( GL_DEPTH_TEST ) ;

  glFogf ( GL_FOG_DENSITY, 3.0f / 100.0f ) ;
  glFogfv( GL_FOG_COLOR  , skycol ) ;
  glFogf ( GL_FOG_START  , 0.0       ) ;
  glFogi ( GL_FOG_MODE   , GL_EXP    ) ;
  glHint ( GL_FOG_HINT   , GL_NICEST ) ;

  sgCoord cam ;
  sgSetVec3 ( cam.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( cam.hpr, 0, 0, 0 ) ;
  ssgSetCamera ( & cam ) ;

  glEnable ( GL_FOG ) ;
  ssgCullAndDraw ( scene ) ;
  glDisable ( GL_FOG ) ;
}


void GFX::done ()
{
  glutPostRedisplay () ;
  glutSwapBuffers () ;
}


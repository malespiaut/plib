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

#include <plib/pu.h>

//#define VOODOO 1

void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
  glutPostRedisplay () ;
}

void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
  glutPostRedisplay () ;
}

void displayfn ( void )
{
  glClearColor ( 0.1f, 0.4f, 0.1f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT ) ;

  puDisplay () ;

  glutSwapBuffers   () ;

  /* The next line is not neccessary - you could remove it safely without
     affecting the functionality of this simple example program.

     It exists because in every application which does some more stuff
     than creating user interface widgets, you normally do want to
     redraw your scenery as often as possible for smooth animation. */

  glutPostRedisplay () ;
}

void button_cb ( puObject * )
{
  fprintf ( stderr, "Hello World.\n" ) ;
}
 

int main ( int argc, char **argv )
{
#ifdef VOODOO
  glutInitWindowPosition ( 0, 0 ) ;
#endif
  glutInitWindowSize     ( 640, 480 ) ;
  glutInit               ( &argc, argv ) ;

  /* Note that in order for PUI and this example program to work, you
     definitely don't need a depth buffer.

     However, most applications using PUI do some more things than rendering
     PUI widgets. In every "real" program, you usually do need a depth
     buffer - we are requesting one in the next line so that PLIB programmers
     can write their applications upon this example code without running
     into problems. */

  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;

  glutCreateWindow       ( "PUI Application"  ) ;
  glutDisplayFunc        ( displayfn ) ;
  glutMouseFunc          ( mousefn   ) ;
  glutMotionFunc         ( motionfn  ) ;

#ifdef VOODOO
  glutPassiveMotionFunc  ( motionfn  ) ;
#endif

  puInit () ;

#ifdef VOODOO
  puShowCursor () ;
#endif

  puOneShot *b = new puOneShot ( 50, 50, 200, 80 ) ;

  b -> setLegend   ( "Say Hello" ) ;
  b -> setCallback ( button_cb ) ;

printf ( "%d\n", PLIB_VERSION ) ;

  glutMainLoop () ;

  return 0 ;
}


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

#include <plib/fnt.h>

static fntRenderer *text ;

static const char *font_names [] =
{
  "Helvetica.txf",         /* This is the default */
  "Helvetica-Bold.txf",
  "Helvetica-Oblique.txf",
  "Helvetica-BoldOblique.txf",

  "AvantGarde-Book.txf",
  "AvantGarde-BookOblique.txf",
  "AvantGarde-Demi.txf",
  "AvantGarde-DemiOblique.txf",

  "Bookman-Light.txf",
  "Bookman-LightItalic.txf",
  "Bookman-Demi.txf",
  "Bookman-DemiItalic.txf",

  "Courier.txf",
  "Courier-Bold.txf",
  "Courier-Oblique.txf",
  "Courier-BoldOblique.txf",

  "NewCenturySchlbk-Roman.txf",
  "NewCenturySchlbk-Italic.txf",
  "NewCenturySchlbk-Bold.txf",
  "NewCenturySchlbk-BoldItalic.txf",

  "Palatino-Roman.txf",
  "Palatino-Italic.txf",
  "Palatino-Bold.txf",
  "Palatino-BoldItalic.txf",

  "Times-Roman.txf",
  "Times-Italic.txf",
  "Times-Bold.txf",
  "Times-BoldItalic.txf",

  "ZapfChancery-MediumItalic.txf",

  "old/sorority.txf",
  "old/charter.txf", 
/*  "old/courier-bold.txf", 
    "old/courier_medium.txf", */
  "old/curlfont.txf", 
  "old/default.txf", 
  "old/derniere.txf", 
  "old/haeberli.txf", 
/*  "old/helvetica_bold.txf", 
    "old/helvetica_medium.txf", */
  "old/junius.txf", 
  "old/ledfixed.txf", 
  "old/lucida.txf", 
  "old/lucidabright_bold.txf", 
/*  "old/schoolbook_bold.txf", 
    "old/schoolbook_medium.txf", */
  "old/symbol.txf", 
/*  "old/times_bold.txf", 
    "old/times_medium.txf",
    "old/typewriter.txf", */
  NULL
} ;

static fntTexFont **font_list ;

static int cur_font = 0 ;
static int max_font = 0 ;

static void motionfn ( int, int )
{
  glutPostRedisplay () ;
}

static void keyfn ( unsigned char key, int, int )
{
  cur_font++ ;

  if ( cur_font >= max_font )
    cur_font = 0 ;
}

static void mousefn ( int /*button*/, int /*updown*/, int /*x*/, int /*y*/ )
{
  exit ( 0 ) ;
}

static int getWindowHeight () { return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ; }
static int getWindowWidth  () { return glutGet ( (GLenum) GLUT_WINDOW_WIDTH  ) ; }

static void setOpenGLState ( void )
{
  int w = getWindowWidth  () ;
  int h = getWindowHeight () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glEnable       ( GL_ALPHA_TEST ) ;
  glEnable       ( GL_BLEND ) ;
  glAlphaFunc    ( GL_GREATER, 0.1f ) ;
  glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
 
  glViewport     ( 0, 0, w, h ) ;
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  gluOrtho2D     ( 0, w, 0, h ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
}

static void restoreOpenGLState ( void )
{
  glMatrixMode   ( GL_PROJECTION ) ;
  glPopMatrix    () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPopMatrix    () ;
  glPopAttrib    () ;
}



static void displayfn (void)
{
  setOpenGLState () ;
  glClearColor ( 0.1f, 0.4f, 0.1f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT ) ;

  text -> setFont      ( font_list [ 0 ] ) ;
  text -> setPointSize ( 25.0f ) ;

  text -> begin () ;
    glColor3f ( 1.0f, 1.0f, 0.0f ) ;
    text -> start2f ( 50.0f, 400.0f ) ;
    text -> puts ( "This is the PLIB Font Demo." ) ;
  text -> end () ;

  text -> setFont      ( font_list [ 0 ] ) ;
  text -> setPointSize ( 12.0f ) ;

  text -> begin () ;
    glColor3f ( 0.0f, 0.0f, 0.0f ) ;
    text -> start2f ( 270.0f, 168.0f ) ;
    text -> puts ( "The current font is:" ) ;
    text -> start2f ( 300.0f, 148.0f ) ;
    text -> puts ( font_names [ cur_font ] ) ;
    text -> start2f ( 270.0f, 128.0f ) ;
    text -> puts ( "Press any key to change the font.\nClick the mouse to exit." ) ;
  text -> end () ;

  text -> setFont      ( font_list [ cur_font ] ) ;
  text -> setPointSize ( 25.0f ) ;

  text -> begin () ;
    glColor3f ( 0.0f, 1.0f, 1.0f ) ;
    text -> start2f ( 50.0f, 360.0f ) ;
    text -> puts ( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) ;
    glColor3f ( 1.0f, 0.0f, 1.0f ) ;
    text -> start2f ( 50.0f, 320.0f ) ;
    text -> puts ( "abcdefghijklmnopqrstuvwxyz" ) ;
    glColor3f ( 1.0f, 0.0f, 0.0f ) ;
    text -> start2f ( 50.0f, 280.0f ) ;
    text -> puts ( "0123456789!@#$%^&*()+-={}[]:\"'<>?,./`~_" ) ;
  text -> end () ;

  glDisable ( GL_TEXTURE_2D ) ;
  glBegin ( GL_LINE_LOOP ) ;
  glColor3f ( 1.0f, 0.0f, 1.0f ) ;
   glVertex2f (   0.0f,   0.0f ) ;
   glVertex2f ( 256.0f,   0.0f ) ;
   glVertex2f ( 256.0f, 256.0f ) ;
   glVertex2f (   0.0f, 256.0f ) ;
  glEnd () ;

  glEnable ( GL_TEXTURE_2D ) ;
  glBegin ( GL_TRIANGLE_STRIP ) ;
  glColor3f ( 1.0f, 1.0f, 1.0f ) ;
  glTexCoord2f( 0.0f, 0.0f ) ; glVertex2f (   0.0f,   0.0f ) ;
  glTexCoord2f( 1.0f, 0.0f ) ; glVertex2f ( 256.0f,   0.0f ) ;
  glTexCoord2f( 0.0f, 1.0f ) ; glVertex2f (   0.0f, 256.0f ) ;
  glTexCoord2f( 1.0f, 1.0f ) ; glVertex2f ( 256.0f, 256.0f ) ;
  glEnd () ;

  restoreOpenGLState () ;
  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


int main ( int argc, char **argv )
{
  font_list = new fntTexFont* [ sizeof(font_names) / sizeof(font_names[0]) ] ;

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize  ( 640, 480 ) ;
  glutInit            ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow    ( "FNT Application"  ) ;
  glutDisplayFunc     ( displayfn ) ;
  glutMouseFunc       ( mousefn   ) ;
  glutMotionFunc      ( motionfn  ) ;
  glutKeyboardFunc    ( keyfn     ) ;
  
  text = new fntRenderer () ;

  for ( max_font = 0 ; font_names [ max_font ] != NULL ; max_font++ )
  {
    char fname [ 256 ] ;

#ifdef macintosh
    sprintf ( fname, ":data:%s", font_names [ max_font ] ) ;
#else
    sprintf ( fname, "data/%s", font_names [ max_font ] ) ;
#endif

    font_list [ max_font ] = new fntTexFont ( fname ) ;
  }

  cur_font = 0 ;

  glutMainLoop () ;
  return 0 ;
}


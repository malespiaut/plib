
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <GL/glut.h>
#include <plib/fnt.h>

fntRenderer *text ;

#define MAX_FONTS 20

char *font_names [ MAX_FONTS ] = 
{
  "sorority.txf",          /* This is the default */
  "charter.txf", 
  "courier-bold.txf", 
  "courier_medium.txf", 
  "curlfont.txf", 
  "default.txf", 
  "haeberli.txf", 
  "helvetica_bold.txf", 
  "helvetica_medium.txf", 
  "lucida.txf", 
  "lucidabright_bold.txf", 
  "schoolbook_bold.txf", 
  "schoolbook_medium.txf", 
  "symbol.txf", 
  "times_bold.txf", 
  "times_medium.txf", 
  "typewriter.txf",
  NULL
} ;

fntTexFont *font_list [ MAX_FONTS ] ;

int cur_font = 0 ;
int max_font = 0 ;

void motionfn ( int, int )
{
  glutPostRedisplay () ;
}

void keyfn ( unsigned char key, int, int )
{
  cur_font++ ;

  if ( cur_font >= max_font )
    cur_font = 0 ;
}

void mousefn ( int /*button*/, int /*updown*/, int /*x*/, int /*y*/ )
{
  exit ( 0 ) ;
}

int getWindowHeight () { return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ; }
int getWindowWidth  () { return glutGet ( (GLenum) GLUT_WINDOW_WIDTH  ) ; }

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



void displayfn (void)
{
  setOpenGLState () ;
  glClearColor ( 0.1f, 0.4f, 0.1f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT ) ;

  text -> setFont      ( font_list [ 0 ] ) ;
  text -> setPointSize ( 25 ) ;

  text -> begin () ;
    glColor3f ( 1, 1, 0 ) ;
    text -> start2f ( 50, 400 ) ;
    text -> puts ( "This is the PLIB Font Demo." ) ;
  text -> end () ;

  text -> setFont      ( font_list [ 0 ] ) ;
  text -> setPointSize ( 12 ) ;

  text -> begin () ;
    glColor3f ( 0, 0, 0 ) ;
    text -> start2f ( 270, 168 ) ;
    text -> puts ( "The current font is:" ) ;
    text -> start2f ( 300, 148 ) ;
    text -> puts ( font_names [ cur_font ] ) ;
    text -> start2f ( 270, 128 ) ;
    text -> puts ( "Press any key to change the font.\nClick the mouse to exit." ) ;
  text -> end () ;

  text -> setFont      ( font_list [ cur_font ] ) ;
  text -> setPointSize ( 25 ) ;

  text -> begin () ;
    glColor3f ( 0, 1, 1 ) ;
    text -> start2f ( 50, 360 ) ;
    text -> puts ( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) ;
    glColor3f ( 1, 0, 1 ) ;
    text -> start2f ( 50, 320 ) ;
    text -> puts ( "abcdefghijklmnopqrstuvwxyz" ) ;
    glColor3f ( 1, 0, 0 ) ;
    text -> start2f ( 50, 280 ) ;
    text -> puts ( "0123456789!@#$%^&*()+-={}[]:\"'<>?,./`~_" ) ;
  text -> end () ;

  glDisable ( GL_TEXTURE_2D ) ;
  glBegin ( GL_LINE_LOOP ) ;
  glColor3f ( 1, 0, 1 ) ;
   glVertex2f (   0,   0 ) ;
   glVertex2f ( 256,   0 ) ;
   glVertex2f ( 256, 256 ) ;
   glVertex2f (   0, 256 ) ;
  glEnd () ;

  glEnable ( GL_TEXTURE_2D ) ;
  glBegin ( GL_TRIANGLE_STRIP ) ;
  glColor3f ( 1, 1, 1 ) ;
  glTexCoord2f( 0, 0 ) ; glVertex2f (   0,   0 ) ;
  glTexCoord2f( 1, 0 ) ; glVertex2f ( 256,   0 ) ;
  glTexCoord2f( 0, 1 ) ; glVertex2f (   0, 256 ) ;
  glTexCoord2f( 1, 1 ) ; glVertex2f ( 256, 256 ) ;
  glEnd () ;

  restoreOpenGLState () ;
  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


int main ( int argc, char **argv )
{
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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#  include <windows.h>
#  ifdef __CYGWIN32__
#    include <sys/stat.h>
#    include <unistd.h>
#  else
#    include <sys/stat.h>
#  endif
#else
#  include <sys/stat.h>
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
static fntTexFont *font ;

static char  *file ;
static char **line ;
static int   *page ;
static int  num_lines = 0 ;
static int  num_pages = 0 ;
static int  curr_page = 0 ;

static void motionfn ( int, int )
{
  glutPostRedisplay () ;
}

static void keyfn ( unsigned char key, int, int )
{
  switch ( key )
  {
    case 'H' : case 'R' : curr_page = 0 ; break ;

    case '<' : case '{' : curr_page-=5 ; break ;

    case 0x08 :  /* Backspace */
    case '[' : case ',' : curr_page-- ; break ;

    case '>' : case '}' : curr_page+=5 ; break ;
    case ' ' :
    case '\n' :
    case '\r' :
    case ']' : case '.' : curr_page++ ; break ;

    case 0x03 : /* Ctrl-C */
    case 'x' :
    case 'X' : exit ( 0 ) ;
  }

  if ( curr_page < 0 ) curr_page = 0 ;
  if ( curr_page >= num_pages ) curr_page = num_pages-1 ;
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

  glutWarpPointer ( 320, 240 ) ;

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

  text -> setFont      ( font ) ;
  text -> setPointSize ( 25.0f ) ;

  int row = 460 ;

  text -> begin () ;
    glColor3f ( 1.0f, 1.0f, 1.0f ) ;

    for ( int i = page [ curr_page ] ;
                  i < page [ curr_page+1 ]-1 && i < num_lines && row > 0 ; i++ )
    {
      text -> start2f ( 10.0f, row ) ;
      text -> puts ( line[i] ) ;
      row -= 30 ;
    }
  text -> end () ;

  restoreOpenGLState () ;
  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


int main ( int argc, char **argv )
{

  struct stat buf ;
  const char* fname = NULL ;

  if ( argc > 1 )
     fname = argv[1] ;
  else
     fname = "data/test_slideshow.txt" ;

  FILE *fd = fopen ( fname, "r" ) ;

  if ( fd == NULL )
    exit ( 1 ) ;
 
  fstat ( fileno(fd), & buf ) ;

  off_t len = buf.st_size ;

  file = new char  [ len+1 ] ;
  line = new char* [ len+1 ] ;
  page = new int   [ len+1 ] ;

  fread ( file, 1, len, fd ) ;
  fclose ( fd ) ;

  file [ len ] = '\0' ;

  num_lines = 0 ;
  num_pages = 0 ;

  line [ num_lines ] = file ;
  page [ num_pages ] = 0 ;

  for ( int i = 0 ; i < len ; i++ )
  {
    if ( file [ i ] == '\n' )
    {
      file [ i ]  = '\0' ;
      line [ ++num_lines ] = & file [ i+1 ] ;

      if ( line [ num_lines ][ 0 ] == '*' )
        page [ ++num_pages ] = num_lines+1 ;
    }
  }

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
  font = new fntTexFont ( "data/old/lucida.txf" ) ;

  glutMainLoop () ;
  return 0 ;
}



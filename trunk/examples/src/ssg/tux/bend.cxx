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

ssgRoot      *scene    = NULL ;


/*
  The GLUT window reshape event
*/

void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}



/*
  The GLUT keyboard event
*/

void keyboard ( unsigned char, int, int )
{
  exit ( 0 ) ;
}



/*
  The GLUT redraw event
*/

void redraw ()
{
  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  ssgCullAndDraw ( scene ) ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}



void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "ssgExample" ;
  fake_argv[1] = "Simple Scene Graph : Example Program." ;
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
  ssgSetNearFar ( 1.0f, 10000.0f ) ;

  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;

  /*
    Set up the Camera.
  */

  sgCoord campos ;
  sgSetCoord ( & campos, 0.0f, -1000.0f, 200.0f, 0.0f, 0.0f, 0.0f ) ;
  ssgSetCamera ( & campos ) ;
}


/*
  Load a simple database
*/

void load_database ()
{
  /*
    Set up the path to the data files
  */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /*
    Create a root node
  */

  scene    = new ssgRoot      ;

  /*
    Load the model and then add it to the scene.
  */

  ssgEntity *object = ssgLoadASE ( "bend.ase" ) ;
  scene -> addKid ( object ) ;

  ssgEntity* found = object -> getByName ( "Cylinder01" );
  if ( found && found -> isAKindOf ( SSG_TYPE_TIMEDSELECTOR ) )
  {
    ssgTimedSelector* p = (ssgTimedSelector*) found;
    p -> setMode ( SSG_ANIM_SWING ) ;
    p -> control ( SSG_ANIM_START ) ;
    p -> setDuration ( 30.0f ) ;
  }
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




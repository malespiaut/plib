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
ssgTransform *penguin  = NULL ;
ssgTransform *pedestal = NULL ;


/*
  Something to make some interesting motion
  for both Tux and the camera.
*/

void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;
  sgCoord tuxpos ;

  /*
    Spin Tux, make the camera pan sinusoidally left and right
  */

  sgSetCoord ( & campos, 0.0f, -5.0f, 1.0f, 25.0 * sin(frameno/100.0), 0.0f, 0.0f ) ;
  sgSetCoord ( & tuxpos, 0.0f,  0.0f, 0.0f, frameno, 0.0f, 0.0f ) ;

  ssgSetCamera ( & campos ) ;
  penguin -> setTransform ( & tuxpos ) ;
}



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
  update_motion () ;

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
  ssgSetNearFar ( 1.0f, 700.0f ) ;

  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
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
    Create a root node - and a transform to position
    the pedestal - and another to position the penguin
    beneath that (in the tree that is).
  */

  scene    = new ssgRoot      ;
  pedestal = new ssgTransform ;
  penguin  = new ssgTransform ;

  /*
    Load the models - optimise them a bit
    and then add them into the scene.
  */

  ssgEntity *ped_obj = ssgLoadAC ( "pedestal.ac" ) ;
  ssgEntity *tux_obj = ssgLoadAC ( "tuxedo.ac"   ) ;

  penguin  -> addKid ( tux_obj  ) ;
  pedestal -> addKid ( ped_obj  ) ;

  ssgFlatten         ( tux_obj  ) ;
  ssgFlatten         ( ped_obj  ) ;
  ssgStripify        ( penguin  ) ;
  ssgStripify        ( pedestal ) ;

  pedestal -> addKid ( penguin  ) ;
  scene    -> addKid ( pedestal ) ;
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




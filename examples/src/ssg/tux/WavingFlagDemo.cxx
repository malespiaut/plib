#include <GL/glut.h>
#include "plib/ssg.h"
#include "WavingFlag.h"


static WavingFlag* flag = NULL ;
static float flagTime = 0.0f ;
static ssgRoot* scene = NULL ;

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



void idle ()
{
  static int lastTime = 0;
  int time = glutGet(GLUT_ELAPSED_TIME);
  if (time != lastTime) {
    lastTime = time;
    flagTime += 0.02f ;
  }
}


/*
  The GLUT redraw event
*/

void redraw ()
{
  sgCoord campos ;
  sgSetCoord ( & campos, 0.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f ) ;
  ssgSetCamera ( & campos ) ;

  flag -> animate( flagTime, 20 );

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
  fake_argv[1] = "Waving Flag Example Program." ;
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
  glutIdleFunc           ( idle ) ;
 
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
    Create a root node and the flag.
  */

  scene    = new ssgRoot ;

  sgVec4 flagColor = { 1.0f, 0.0f, 0.0f, 1.0f };
  flag = new WavingFlag ( flagColor, "data/Penguin_body.rgb", "data/Penguin_body.rgb" ) ;

  scene -> addKid ( flag -> getObject() ) ;
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

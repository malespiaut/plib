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
#include <plib/ssgAux.h>
#include <GL/glut.h>

ssgRoot        *scene     = NULL ;
ssgTransform   *penguin   = NULL ;
ssgTransform   *pedestal  = NULL ;
ssgaWaveSystem *ocean     = NULL ;
//ssgaCube *ocean     = NULL ;
ssgSimpleState *sea_state = NULL ;


/*
  Something to make some interesting motion
  for both Tux and the camera.
*/

float getDepth ( float x, float y )
{
  return (x > 0.0f) ? fabs ( sin(x/15.0f) * sin(y/10.0f) * 0.5 + 0.5 ) : 1.0f;
}


void update_motion ( int frameno )
{
  sgCoord campos ;
  sgCoord tuxpos ;

  /*
    Spin Tux, make the camera pan sinusoidally left and right
  */

  if ( frameno < 600 )
    sgSetCoord ( & campos, 0.0f, -20.0f, 8.0f,
                         25.0f * (float) sin(frameno/100.0), -30.0f, 0.0f ) ;
  else
    sgSetCoord ( & campos, 0.0f, 0.0f, 2.0f,
                         frameno, -10.0f, 0.0f ) ;

  sgSetCoord ( & tuxpos, 0.0f,  0.0f, 0.6f, frameno, 0.0f, 0.0f ) ;

  ssgSetCamera ( & campos ) ;
  penguin -> setTransform ( & tuxpos ) ;

  ocean -> setWindDirn ( 25.0 * sin ( frameno / 100.0 ) ) ;

  static float dt = 0.0f ;

  dt += 0.016 ;

// sgVec3 center ;
// sgSetVec3 ( center, ( frameno % 100 ) / 10.0f,
//                     ( frameno % 200 ) / 20.0f, 0.0f ) ;
// ocean -> setCenter ( center ) ;

  ocean -> updateAnimation ( dt ) ;
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
static int frameno = 0 ;
frameno++ ;

  update_motion ( frameno % 2000 ) ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

if ( frameno % 2000 > 1900 )
  glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
else
  glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;

  ssgCullAndDraw ( scene ) ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
  usleep ( 15000 ) ;
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
  sgSetVec3 ( sunposn, 0.1f, -1.0f, 0.1f ) ;
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

  scene     = new ssgRoot      ;
  pedestal  = new ssgTransform ;
  penguin   = new ssgTransform ;
  ocean     = new ssgaWaveSystem ( 10000 ) ;
//  ocean     = new ssgaCube ( 20000 ) ;
  sea_state = new ssgSimpleState () ;
  sea_state -> setTexture        ( "data/ocean.rgb" ) ;
  sea_state -> enable            ( GL_TEXTURE_2D ) ;
  sea_state -> setShadeModel     ( GL_SMOOTH ) ;
  sea_state -> enable            ( GL_CULL_FACE ) ;
  sea_state -> disable           ( GL_BLEND ) ;
  sea_state -> enable            ( GL_LIGHTING ) ;
  sea_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  sea_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  sea_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  sea_state -> setShininess      (  5 ) ;

  sgVec4 SEABLUE = { 1.0, 1.0, 1.0, 1.0 } ;
  sgVec3 pos = { 0, 0, 0 } ;

  ocean -> setColour   ( SEABLUE ) ;
  ocean -> setSize     ( 50 ) ;
  ocean -> setTexScale ( 6, 6 ) ;
  ocean -> setCenter   ( pos ) ;
  ocean -> setDepthCallback ( getDepth ) ;
  ocean -> setKidState ( sea_state ) ;
  ocean -> setWaveHeight ( 0.3 ) ;
  ocean -> setWindSpeed  ( 10.0f ) ;
  ocean -> regenerate  () ;

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

  scene    -> addKid ( ocean ) ;
  scene    -> addKid ( pedestal ) ;
  pedestal -> addKid ( penguin  ) ;
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




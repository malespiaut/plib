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

ssgRoot            *scene        = NULL ;
ssgTransform       *teapot       = NULL ;
ssgTransform       *pedestal     = NULL ;
ssgaWaveSystem     *ocean        = NULL ;
ssgaParticleSystem *fountain     = NULL ;
ssgaCube           *ped_obj      = NULL ;
ssgaTeapot         *tpt_obj      = NULL ;

ssgSimpleState     *sea_state    = NULL ;
ssgSimpleState     *splash_state = NULL ;
ssgSimpleState     *teapot_state = NULL ;
ssgSimpleState     *plinth_state = NULL ;


float getDepth ( float x, float y )
{
  return (x > 0.0f) ? fabs ( sin(x/15.0f) * sin(y/10.0f) * 0.5 + 0.5 ) : 1.0f;
}


void update_motion ( int frameno )
{
  sgCoord campos ;
  sgCoord tptpos ;

  /* Move the camera in some kind of interesting way */

  if ( frameno < 600 )
    sgSetCoord ( & campos, 0.0f, -20.0f, 8.0f,
                         25.0f * (float) sin(frameno/100.0), -30.0f, 0.0f ) ;
  else
    sgSetCoord ( & campos, 0.0f, 0.0f, 2.0f,
                         frameno, -10.0f, 0.0f ) ;

  sgSetCoord ( & tptpos, 0.0f,  0.0f, 0.6f, frameno, 0.0f, 0.0f ) ;

  ssgSetCamera ( & campos ) ;
  teapot  -> setTransform ( & tptpos ) ;

  ocean -> setWindDirn ( 25.0 * sin ( frameno / 100.0 ) ) ;

  static ulClock ck ; ck . update () ;

  float t  = ck . getAbsTime   () ;
  float dt = ck . getDeltaTime () ;

// sgVec3 center ;
// sgSetVec3 ( center, ( frameno % 100 ) / 10.0f,
//                     ( frameno % 200 ) / 20.0f, 0.0f ) ;
// ocean -> setCenter ( center ) ;

  ocean -> updateAnimation ( t ) ;
  fountain -> update ( dt ) ;
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
  Particle system definitions for the fountain.
*/

#define SPS ssgaParticleSystem  /* Too much typing! */                          

static void droplet_create ( SPS *, int, ssgaParticle *p )
{
  float c = 0.6 + (float)(rand()%1000)/4000.0f ;

  sgSetVec4 ( p -> col, c - 0.2f, c, 1, 0.5 ) ;
  sgSetVec3 ( p -> pos, -2.5, 0, 2 ) ;
  sgSetVec3 ( p -> vel, 
             -(float)(rand()%1000)/200.0f,
              (float)(rand()%1000 - 500)/400.0f,
              (float)(rand()%1000)/1000.0f + 3.0f ) ;
  sgAddScaledVec3 ( p -> pos, p -> vel, (float)(rand()%1000)/20000.0f ) ;
  sgSetVec3 ( p -> acc, 0, 0, -9.8 ) ;
  p -> time_to_live = 1 ;
}


static void droplet_update ( float dt, SPS *, int, ssgaParticle *p )
{
  sgAddScaledVec3 ( p->vel, p->acc, dt ) ;
  sgAddScaledVec3 ( p->pos, p->vel, dt ) ;
}


void init_states ()
{
  plinth_state = new ssgSimpleState () ;
  plinth_state -> setTexture        ( "data/pavement.rgb" ) ;
  plinth_state -> enable            ( GL_TEXTURE_2D ) ;
  plinth_state -> setShadeModel     ( GL_SMOOTH ) ;
  plinth_state -> enable            ( GL_CULL_FACE ) ;
  plinth_state -> enable            ( GL_BLEND ) ;
  plinth_state -> enable            ( GL_LIGHTING ) ;
  plinth_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  plinth_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  plinth_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  plinth_state -> setShininess      ( 2 ) ;

  teapot_state = new ssgSimpleState () ;
  teapot_state -> setTexture        ( "data/pattern.rgb" ) ;
  teapot_state -> enable            ( GL_TEXTURE_2D ) ;
  teapot_state -> setShadeModel     ( GL_SMOOTH ) ;
  teapot_state -> enable            ( GL_CULL_FACE ) ;
  teapot_state -> enable            ( GL_BLEND ) ;
  teapot_state -> enable            ( GL_LIGHTING ) ;
  teapot_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  teapot_state -> setMaterial       ( GL_EMISSION, 0, 0, 0, 1 ) ;
  teapot_state -> setMaterial       ( GL_SPECULAR, 1, 1, 1, 1 ) ;
  teapot_state -> setShininess      ( 2 ) ;

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

  splash_state = new ssgSimpleState () ;
  splash_state -> setTexture        ( "data/droplet.rgb" ) ;
  splash_state -> enable            ( GL_TEXTURE_2D ) ;
  splash_state -> setShadeModel     ( GL_SMOOTH ) ;
  splash_state -> enable            ( GL_CULL_FACE ) ;
  splash_state -> enable            ( GL_BLEND ) ;
  splash_state -> enable            ( GL_LIGHTING ) ;
  splash_state -> setColourMaterial ( GL_EMISSION ) ;
  splash_state -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  splash_state -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  splash_state -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  splash_state -> setShininess      (  0 ) ;
}



void load_database ()
{
  /* Set up the path to the data files */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /* Load the states */

  init_states () ;

  sgVec4  WHITE  = { 1.0, 1.0, 1.0, 1.0 } ;
  sgVec3  pos    = { 0, 0, 0 } ;
  sgCoord pedpos = { { 0, 0, -1.5 }, { 0, 0, 0 } } ;

  /* Create a the scene content.  */

  fountain = new ssgaParticleSystem ( 1000, 100, 500, TRUE,
                                      0.2, 1000,
                                      droplet_create,
                                      droplet_update, NULL ) ;
  fountain -> setState ( splash_state ) ;

  ocean   =  new ssgaWaveSystem ( 10000 ) ;
  ocean   -> setColour        ( WHITE ) ;
  ocean   -> setSize          ( 50 ) ;
  ocean   -> setTexScale      ( 6, 6 ) ;
  ocean   -> setCenter        ( pos ) ;
  ocean   -> setDepthCallback ( getDepth ) ;
  ocean   -> setKidState      ( sea_state ) ;
  ocean   -> setWaveHeight    ( 0.3 ) ;
  ocean   -> setWindSpeed     ( 10.0f ) ;
  ocean   -> regenerate       () ;

  ped_obj =  new ssgaCube     () ;
  ped_obj -> setSize          ( 4 ) ;
  ped_obj -> setKidState      ( plinth_state ) ;
  ped_obj -> regenerate       () ;

  tpt_obj =  new ssgaTeapot   ( 1000 ) ;
  tpt_obj -> setSize          ( 2 ) ;
  tpt_obj -> setKidState      ( teapot_state ) ;
  tpt_obj -> regenerate       () ;

  /* Build the scene graph */

  teapot   =  new ssgTransform ;
  teapot   -> addKid          ( tpt_obj  ) ;
  teapot   -> addKid          ( fountain ) ;

  pedestal =  new ssgTransform ;
  pedestal -> setTransform    ( & pedpos ) ;
  pedestal -> addKid          ( ped_obj  ) ;

  scene    =  new ssgRoot ;
  scene    -> addKid          ( ocean    ) ;
  scene    -> addKid          ( pedestal ) ;
  scene    -> addKid          ( teapot   ) ;
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




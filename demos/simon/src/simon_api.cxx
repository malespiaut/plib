
#include "simon.h"
#include <plib/js.h>

class siEntity ;

static int       siNextModel   =   0   ;
static bool      siInitialised = false ;
static ssgRoot  *siScene       = NULL  ;
static siEntity *siModels [ SI_MAX_MODELS ] ;

class siEntity
{
protected:

  int        handle ;
  char        *name ;
  sgCoord       pos ;
  sgCoord       vel ;

public:

  siEntity ( char *_fname )
  {
    siModels [ siNextModel ] = this ;

    handle = siNextModel++ ;

    if ( siNextModel >= SI_MAX_MODELS )
    {
      fprintf ( stderr, "simon: ERROR - More than %d models loaded!\n", SI_MAX_MODELS ) ;
      exit ( 1 ) ;
    }

    name = new char [ strlen(_fname)+1 ] ;
    strcpy ( name, _fname ) ;
    sgZeroCoord ( & pos ) ;
    sgZeroCoord ( & vel ) ;
  }

  int getHandle () { return handle ; }

  float getPositionX () { return pos.xyz[0] ; }
  float getPositionY () { return pos.xyz[1] ; }
  float getPositionZ () { return pos.xyz[2] ; }
  float getPositionH () { return pos.hpr[0] ; }
  float getPositionP () { return pos.hpr[1] ; }
  float getPositionR () { return pos.hpr[2] ; }

  void setSpeedAndDirection ( float s, float h, float p ) 
  {
    sgMat4 tmp ;
    sgMakeRotMat4 ( tmp, h, p, 0 ) ;

    sgSetVec3   ( vel.xyz, 0, s, 0 ) ;
    sgXformPnt3 ( vel.xyz, tmp ) ;
    sgSetVec3   ( pos.hpr, h, p, 0 ) ;
    sgZeroVec3  ( vel.hpr ) ;
  }

  void setVelocity ( float x, float y, float z, float h, float p, float r ) 
  {
    sgSetCoord ( &vel, x, y, z, h, p, r ) ;
  }

  void setPosition ( float x, float y, float z, float h, float p, float r ) 
  {
    sgSetCoord ( &pos, x, y, z, h, p, r ) ;
  }

  void updatePhysics ( double dt )
  {
    sgAddScaledVec3 ( pos . xyz, vel . xyz, dt ) ; 
    sgAddScaledVec3 ( pos . hpr, vel . hpr, dt ) ; 
  }

  virtual void update ( double dt ) = NULL ;
} ;



class siCamera : public siEntity
{
public:

  siCamera () : siEntity ( "Camera" )
  {
  }

  virtual void update ( double dt )
  {
    updatePhysics ( dt ) ;
    ssgSetCamera ( & pos ) ;
  }
} ;



class siModel : public siEntity
{
  ssgEntity    *ent ;
  ssgTransform *tra ;

public:

  siModel ( char *_fname ) : siEntity ( _fname )
  {
    tra = new ssgTransform ;
    ent = ssgLoad ( name ) ;
    tra -> addKid ( ent ) ;

    ssgFlatten  ( ent ) ;
    ssgStripify ( tra ) ;

    siScene -> addKid ( tra ) ;
  }

  virtual void update ( double dt )
  {
    updatePhysics ( dt ) ;
    tra -> setTransform ( & pos ) ;
  }
} ;


siEntity *getModel ( int i )
{
  if ( i < 0 || i >= SI_MAX_MODELS ||
       siModels [ i ] == NULL )
  {
    fprintf ( stderr, "simon: There is no such model as '%d'\n", i ) ;
    exit ( 1 ) ;
  }

  return siModels [ i ] ;
}


float siGetPositionX(int hh) { return getModel ( hh ) -> getPositionX () ; }
float siGetPositionY(int hh) { return getModel ( hh ) -> getPositionY () ; }
float siGetPositionZ(int hh) { return getModel ( hh ) -> getPositionZ () ; }
float siGetPositionH(int hh) { return getModel ( hh ) -> getPositionH () ; }
float siGetPositionP(int hh) { return getModel ( hh ) -> getPositionP () ; }
float siGetPositionR(int hh) { return getModel ( hh ) -> getPositionR () ; }


void siPosition ( int hh, float x, float y, float z,
                          float h, float p, float r )
{
  getModel ( hh ) -> setPosition ( x, y, z, h, p, r ) ;
}

void siSpeedAndDirection ( int hh, float s, float h, float p ) 
{
  getModel ( hh ) -> setSpeedAndDirection ( s, h, p ) ;
}

void siVelocity ( int hh, float x, float y, float z,
                          float h, float p, float r ) 
{
  getModel ( hh ) -> setVelocity ( x, y, z, h, p, r ) ;
}


/*
  The GLUT window reshape event
*/

static void siReshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}



/*
  The GLUT keyboard event
*/

static void siKeyboard ( unsigned char k, int, int )
{
  if ( k == 0x03 ) exit ( 0 ) ;
}



jsJoystick js (0) ;

bool A, B, C, D, L, R ;

float leftright ;
float updown    ;
 
static void read_joystick ()
{
  int b ;
  float ax [ 10 ] ;

  js.read ( &b, ax ) ;
 
  leftright = ax[0] ;
  updown    = ax[1] ;

  A = (( b &  1 ) != 0) ;
  B = (( b &  2 ) != 0) ;
  C = (( b &  4 ) != 0) ;
  D = (( b &  8 ) != 0) ;
  R = (( b & 16 ) != 0) ;
  L = (( b & 32 ) != 0) ;
}
 
float siJoystickLR () { return -leftright ; }
float siJoystickUD () { return -updown    ; }
 
bool  siJoystickA () { return A ; }
bool  siJoystickB () { return B ; }
bool  siJoystickC () { return C ; }
bool  siJoystickD () { return D ; }
bool  siJoystickL () { return L ; }
bool  siJoystickR () { return R ; }

/*
  The GLUT redraw event
*/

static ulClock *siFrameTimer ;

static void siRedraw ()
{
  siFrameTimer -> update () ;

  double dt = siFrameTimer -> getDeltaTime () ;

  read_joystick () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  siUpdate () ;

  for ( int i = 0 ; i < siNextModel ; i++ )
    siModels [ i ] -> update ( dt ) ;

  ssgCullAndDraw ( siScene ) ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
  usleep(16000);
}



void siInit ()
{
  if ( siInitialised )
  {
    fprintf ( stderr, "simon: ERROR - siInit() was called more than once!\n" ) ;
    exit ( 1 ) ;
  }

  siInitialised = true ;

  siFrameTimer = new ulClock ;

  siFrameTimer -> update () ;

  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "Simon" ;
  fake_argv[1] = "Simon Window." ;
  fake_argv[2] = NULL ;

  /* Initialise GLUT */

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 600, 600 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( siRedraw   ) ;
  glutReshapeFunc        ( siReshape  ) ;
  glutKeyboardFunc       ( siKeyboard ) ;
 
  /* Initialise SSG */

  ssgInit () ;

  /* Some basic OpenGL setup */

  glClearColor ( 0.2f, 0.7f, 1.0f, 1.0f ) ;
  glEnable ( GL_DEPTH_TEST ) ;

  /* Set up the viewing parameters */

  ssgSetFOV     ( 90.0f, 90.0f ) ;
  ssgSetNearFar ( 0.1f, 700.0f ) ;

  /* Set up the Sun.  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;

  /* Set up the path to the data files */ 

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /* Create a root node.  */

  siScene = new ssgRoot ;

  for ( int i = 0 ; i < SI_MAX_MODELS ; i++ )
    siModels [ i ] = NULL ;

  siNextModel = 0 ;
  new siCamera () ;  /* Thus guaranteeing this is handle zero */
}



/* Load a simple model */

int siLoad ( char *filename )
{
  if ( ! siInitialised )
    siInit () ;

  siModel *m = new siModel ( filename ) ;

  return m -> getHandle () ;
}


void siRun ()
{
  if ( ! siInitialised )
    siInit () ;

  glutMainLoop () ; 
}

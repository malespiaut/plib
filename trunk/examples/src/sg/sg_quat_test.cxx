/*
  This is a demonstration of Quaternion vs. Euler rotation

  The cube on the left uses Euler rotation. The cube on
  the right uses Quaternion rotation. As you can see the
  Euler cube is in midst of a gimble-lock-death-throw,
  while the Quaternion cube is happily spinning away.

  I have setup two rows of keys to control the cubes rotations.

  Quaternion Rotation Toggles:
  q - toggle X axis rotation
  w - toggle Y axis rotation
  e - toggle Z axis rotation
  r - reset quaternion to initial state
  t - untoggle all rotations

  Euler Rotation Toggles:
  a - toggle X axis rotation
  s - toggle Y axis rotation
  d - toggle Z axis rotation
  f - reset euler to initial state
  g - untoggle all rotations

  ESC exits

  Written by Michael Kurth (kurth@futurekill.com) (negative0@earthlink.net)
*/
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <plib/sg.h>

#define XSIZE 320
#define YSIZE 320

static sgVec3 Euler ;
static sgVec3 EulerPos ;
static sgVec3 EulerRot ;

static sgVec3 Quat ;
static sgVec3 QuatPos ;
static sgQuat QuatRot ;

static const GLfloat cube_vertices[] =
{
 -1.0, -1.0,  1.0,  1.0, 0.0, 0.0,
  1.0, -1.0,  1.0,  0.5, 0.5, 0.0,
  1.0,  1.0,  1.0,  0.0, 1.0, 0.0,
 -1.0,  1.0,  1.0,  0.0, 0.5, 0.5,
 -1.0, -1.0, -1.0,  0.0, 0.0, 1.0,
  1.0, -1.0, -1.0,  0.5, 0.0, 0.5,
  1.0,  1.0, -1.0,  1.0, 0.0, 0.0,
 -1.0,  1.0, -1.0,  0.5, 0.5, 0.0
} ;

static const GLubyte frontIndices[] = { 0, 1, 2, 3 } ;
static const GLubyte backIndices[] = { 5, 4, 7, 6 } ;

static const GLubyte rightIndices[] = { 1, 5, 6, 2 } ;
static const GLubyte leftIndices[] = { 4, 0, 3, 7 } ;

static const GLubyte topIndices[] = { 3, 2, 6, 7 } ;
static const GLubyte bottomIndices[] = { 4, 5, 1, 0 } ;

static void InitEulerCube( )
{
  sgSetVec3( Euler, 1, 1, 1 ) ;
  sgSetVec3( EulerRot, 0, 0, 0 ) ;
  sgSetVec3( EulerPos, -2, 0, -5 ) ;
}

static void RotateEulerCube( )
{
  if( Euler[ SG_X ] )
  {
    EulerRot[ SG_X ] += 5.0 ;
    if( EulerRot[ SG_X ] > 360. )
      EulerRot[ SG_X ] = 0.0 ;
  }
  
  if( Euler[ SG_Y ] )
  {
    EulerRot[ SG_Y ] += 5.0 ;
    if( EulerRot[ SG_Y ] > 360. )
      EulerRot[ SG_Y ] = 0.0 ;
  }
  
  if( Euler[ SG_Z ] )
  {
    EulerRot[ SG_Z ] += 5.0 ;
    if( EulerRot[ SG_Z ] > 360. )
      EulerRot[ SG_Z ] = 0.0 ;
  }
  
}

static void InitQuatCube( )
{
  sgSetVec3( Quat, 1, 1, 1 ) ;
  sgMakeIdentQuat( QuatRot ) ;
  sgSetVec3( QuatPos, 2, 0, -5 ) ;
}

static void RotateQuatCube( )
{
  if( Quat[ SG_X ] )
    sgRotQuat( QuatRot, 5.0, 1.0, 0.0, 0.0 ) ;
  
  if( Quat[ SG_Y ] )
    sgRotQuat( QuatRot, 5.0, 0.0, 1.0, 0.0 ) ;
  
  if( Quat[ SG_Z ] )
    sgRotQuat( QuatRot, 5.0, 0.0, 0.0, 1.0 ) ;
}


static void DrawCube( )
{
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, frontIndices ) ;
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, backIndices ) ;
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, rightIndices ) ;
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, leftIndices ) ;
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, topIndices ) ;
  glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, bottomIndices ) ;
}

static void Redisplay( void )
{
  sgMat4 matrix ;
  
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  
  glLoadIdentity( ) ;
  glTranslatef( 0, 0, -5 ) ;
  
  glPushMatrix( ) ;
  {
    glTranslatef( QuatPos[ SG_X ], QuatPos[ SG_Y ], QuatPos[ SG_Z ] ) ;
    
    sgQuatToMatrix( matrix, QuatRot ) ;
    glMultMatrixf( (float*)matrix ) ;
    
    DrawCube( ) ;
  }
  glPopMatrix( ) ;
  
  glPushMatrix( ) ;
  {
    glTranslatef( EulerPos[ SG_X ], EulerPos[ SG_Y ], EulerPos[ SG_Z ] ) ;
    
    sgMakeRotMat4( matrix, EulerRot ) ;
    glMultMatrixf( (float*)matrix ) ;
    
    DrawCube( ) ;
  }
  glPopMatrix( ) ;
  
  
  glutSwapBuffers( ) ;
}

static void Idle( )
{
  static int lastTime = 0;
  int time = glutGet((GLenum)GLUT_ELAPSED_TIME);
  if (time > lastTime + 10)
  {
    RotateQuatCube( ) ;
    RotateEulerCube( ) ;
    
    lastTime = time;
  }
  glutPostRedisplay( ) ;
}

static void Keyboard( unsigned char key, int xPos, int yPos )
{
  switch( key )
  {
  case 27:
    exit( 0 ) ;
  case 'q':
    Quat[ SG_X ] = !Quat[ SG_X ] ;
    break ;
  case 'w':
    Quat[ SG_Y ] = !Quat[ SG_Y ] ;
    break ;
  case 'e':
    Quat[ SG_Z ] = !Quat[ SG_Z ] ;
    break ;
  case 'r':
    InitQuatCube( ) ;
    break ;
  case 't':
    sgSetVec3( Quat, 0, 0, 0 ) ;
    break ;
    
  case 'a':
    Euler[ SG_X ] = !Euler[ SG_X ] ;
    break ;
  case 's':
    Euler[ SG_Y ] = !Euler[ SG_Y ] ;
    break ;
  case 'd':
    Euler[ SG_Z ] = !Euler[ SG_Z ] ;
    break ;
  case 'f':
    InitEulerCube( ) ;
    break ;
  case 'g':
    sgSetVec3( Euler, 0, 0, 0 ) ;
    break ;
  }
}

static void Reshape( int w, int h )
{
  
  if( h == 0 )
  {
    h = 1 ;
  }
  
  glViewport( 0, 0, w, h ) ;
  
  glMatrixMode( GL_PROJECTION ) ;
  glLoadIdentity( ) ;
  
  gluPerspective( 45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f ) ;
  
  glMatrixMode( GL_MODELVIEW ) ;
  glLoadIdentity( ) ;
}

static void Init( void )
{
  glClearColor( 0.0, 0.0, 0.0, 0.0 ) ;
  
  glEnable(GL_DEPTH_TEST);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  
  glEnableClientState( GL_VERTEX_ARRAY ) ;
  glEnableClientState( GL_COLOR_ARRAY ) ;
  
  glVertexPointer( 3, GL_FLOAT, 6*sizeof(GLfloat), cube_vertices ) ;
  glColorPointer(  3, GL_FLOAT, 6*sizeof(GLfloat), &cube_vertices[3] ) ;
  
  InitEulerCube( ) ;
  InitQuatCube( ) ;
  
  Reshape( XSIZE, YSIZE ) ;
}

int main( int argc, char **argv )
{
  glutInit( &argc, argv ) ;
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH ) ;
  glutInitWindowSize( XSIZE, YSIZE ) ;
  glutCreateWindow( "Quaternion Demo" ) ;
  
  Init( ) ;
  
  glutDisplayFunc( Redisplay ) ;
  glutReshapeFunc( Reshape ) ;
  glutKeyboardFunc( Keyboard ) ;
  glutIdleFunc( Idle ) ;
  
  glutShowWindow( ) ;
  glutMainLoop( ) ;
  return 0 ;
}


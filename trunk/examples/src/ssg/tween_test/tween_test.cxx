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

ssgRoot            *scene      = NULL ;
ssgTweenController *tween_ctrl = NULL ;


/*
  Something to make some interesting motion
*/

void update_motion ()
{
  static int frameno = 0 ;

  frameno++ ;

  sgCoord campos ;

  /*
    Make the camera pan sinusoidally left and right
    morphing the sphere/spike as we go.
  */

  tween_ctrl -> selectBank ( fabs(sin(frameno/100.0)) ) ;

  sgSetCoord ( & campos, 0.0f, -5.0f, 0.0f, 25.0 * sin(frameno/100.0), 0.0f, 0.0f ) ;
  ssgSetCamera ( & campos ) ;
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
  fake_argv[0] = "ssgTweenExample" ;
  fake_argv[1] = "Simple Scene Graph : Tweening Example Program." ;
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

  sgVec4 black = { 0,0,0,1 } ;
  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, -0.0f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, black ) ;
}


/*
  Load a simple database
*/

void load_database ()
{
  /*
    Create a root node with a Tween controller beneath it.
  */

  scene      = new ssgRoot            ;
  tween_ctrl = new ssgTweenController ;
  scene -> addKid ( tween_ctrl ) ;

  /*
    Construct a sphere (which we'll use as a template
    and then discard
  */

  ssgaSphere *sp = new ssgaSphere ( 100 ) ;

  /*
    For each ssgVtxTable within the sphere, generate
    an ssgTween node
  */

  for ( int i = 0 ; i < sp -> getNumKids () ; i++ )
  {
    ssgVtxTable *vt = (ssgVtxTable *)( sp -> getKid ( i ) ) ;

    assert ( vt -> isAKindOf ( ssgTypeVtxTable () ) ) ;

    /*
      Make two new sets of vertex array.
    */

    ssgVertexArray   *v0 = new ssgVertexArray   ;
    ssgNormalArray   *n0 = new ssgNormalArray   ;
    ssgTexCoordArray *t0 = new ssgTexCoordArray ;
    ssgColourArray   *c0 = new ssgColourArray   ;

    ssgVertexArray   *v1 = new ssgVertexArray   ;
    ssgNormalArray   *n1 = new ssgNormalArray   ;
    ssgTexCoordArray *t1 = new ssgTexCoordArray ;
    ssgColourArray   *c1 = new ssgColourArray   ;

    /*
      For every vertex in the sphere's VtxTable...
    */

    for ( int i = 0 ; i < vt -> getNumVertices () ; i++ )
    {
      /*
        Copy the sphere's vertex (unmolested) into
        the vertex arrays that are destined to become
        bank zero.
      */

      v0 -> add ( vt -> getVertex   (i) ) ;
      n0 -> add ( vt -> getNormal   (i) ) ;
      t0 -> add ( vt -> getTexCoord (i) ) ;
      c0 -> add ( vt -> getColour   (i) ) ;

      /*
        For bank one, make some vertices half
        the usual size - and others twice the
        usual size.  This is a spikey ball.
      */

      sgVec3 vx ;
      sgVec4 co ;

      sgCopyVec3 ( vx, vt -> getVertex (i) ) ;
      sgScaleVec3 ( vx, (i&7) ? 0.5 : 2.0 ) ;

      /*
        Put random colours on the vertices.
      */

      sgSetVec4 ( co, (float)(rand()&0xFF)/255.0f,
                      (float)(rand()&0xFF)/255.0f,
                      (float)(rand()&0xFF)/255.0f, 1.0f ) ;

      /*
        Cheat and make the normals and texture coords be
        the same as the sphere.
      */

      v1 -> add ( vx ) ;
      n1 -> add ( vt -> getNormal   (i) ) ;
      t1 -> add ( vt -> getTexCoord (i) ) ;
      c1 -> add ( co ) ;
    }

    /*
      For each VtxTable in the sphere, create a
      Tween node that morphs between the sphere
      and the spikey sphere.
    */

    ssgTween *tw = new ssgTween ( GL_TRIANGLE_STRIP ) ;
    tw -> newBank ( v0, n0, t0, c0 ) ;
    tw -> newBank ( v1, n1, t1, c1 ) ;

    /*
      Add it into the tween controller
      (although there could be a lot of hierarchy
      between the two if you wanted that)
    */

    tween_ctrl -> addKid ( tw ) ;
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




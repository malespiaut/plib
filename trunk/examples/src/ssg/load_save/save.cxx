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

ssgRoot      *scene   = NULL ;
ssgTransform *object  = NULL ;
ssgEntity    *obj_obj = NULL ;


void redraw ()
{
  return ;
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

  /*
    Initialise SSG
  */

  ssgInit () ;
}


void load_database ()
{
  /*
    Set up the path to the data files
  */

  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;

  /*
    Create a root node - and a transform to position
    the object beneath that (in the tree that is).
  */

  scene  = new ssgRoot      ;
  object = new ssgTransform ;

  /*
    Load the models - optimise them a bit
    and then add them into the scene.
  */

  obj_obj = ssgLoadAC ( "tuxedo.ac" ) ;

  object -> addKid ( obj_obj ) ;

  ssgFlatten       ( obj_obj ) ;
  ssgStripify      ( object  ) ;
  scene -> addKid ( object ) ;
}


void save_database ()
{
  ssgSaveSSG ( "data/tuxedo.ssg", object ) ;
}



int main ( int, char ** )
{
  init_graphics () ;
  load_database () ;
  save_database () ;
  return 0 ;
}


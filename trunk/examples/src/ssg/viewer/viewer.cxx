/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/

/****
* NAME
*   viewer - SSG model viewer
*
* DESCRIPTION
*   using a PUI interface, allow user to load a model,
*   animate the model, and manipulate the camera.
*   several models can be loaded at the same time.
*
* MODIFICATION HISTORY
*   Sep-2000 Dave McClurg <dpm@efn.org> Created
*   Sep-2001 Dave McClurg <dpm@efn.org> Added wireframe toggle
****/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#  include <windows.h>
#else
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

#include <plib/ssg.h>
#include <plib/fnt.h>
#include <plib/pu.h>

#define ARROWS_USED 1
/*
!!!FIXME!!!
Defines what type of arrows to uses with puFileSelector
0 - No arrows
1 - Single move arrows
2 - Single move and Jump 10 moves
*/



/*
scene graph
*/
static ssgRoot *scene = NULL ;
static ssgContext *context = NULL ;
static ssgEntity* camera_object = NULL ;

/*
font vars
*/
static fntRenderer *text ;
static fntTexFont *font ;

static puFileSelector* file_selector = 0 ;

/*
frame rate vars
*/
static int frame_counter = 0;
static float fps = 0;

/*
animation vars
*/
#define MAX_SPEED 60
static const int speed_tab [] =
{ 0, 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60 };

static int speed_index = 1 ;
//static int anim_frame ;
static int num_anim_frames ;

/*
wire vars
*/
static int wire_flag = 0 ;
static sgVec4 wire_colour = { 1.0f, 1.0f, 1.0f, 1.0f } ;

/*
spinner vars
*/
static int downx, downy;   /* for tracking mouse position */
static int downb = -1;     /* and button status */

static GLfloat downDist, downEl, downAz, /* for saving state of things */
downEx, downEy, downEz;   /* when button is pressed */

static GLfloat dAz, dEl, lastAz, lastEl;  /* to calculate spinning w/ polar motion */
static GLfloat AzSpin = 0.0f, ElSpin = 0.0f;
static int     AdjustingAzEl = 0;

/* Minimum spin to allow in polar (lower forced to zero) */
#define MIN_AZSPIN 0.1f
#define MIN_ELSPIN 0.1f

/* Factors used in computing dAz and dEl (which determine AzSpin, ElSpin) */
#define SLOW_DAZ 0.90f
#define SLOW_DEL 0.90f
#define PREV_DAZ 0.80f
#define PREV_DEL 0.80f
#define CUR_DAZ  0.20f
#define CUR_DEL  0.20f

/*
*  polar movement parameters
*/
static GLfloat EyeDist= 100.0f;
static GLfloat EyeAz  = 0.0f;
static GLfloat EyeEl  = 30.0f;
static GLfloat Ex     = 0.0f;
static GLfloat Ey     = 0.0f;
static GLfloat Ez     = 0.0f;

#define FOVY 45.0f
#define NEAR 2.0f
#define FAR  10000.0f

static int getWindowHeight () { return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ; }
static int getWindowWidth  () { return glutGet ( (GLenum) GLUT_WINDOW_WIDTH  ) ; }

static void begin2d ( void )
{
  int w = getWindowWidth  () ;
  int h = getWindowHeight () ;
  
  glDisable      ( GL_LIGHTING ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glEnable       ( GL_ALPHA_TEST ) ;
  glEnable       ( GL_BLEND ) ;
  glAlphaFunc    ( GL_GREATER, 0.1f ) ;
  glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  
  //create an projection that acts like a 2D screen
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  glOrtho        ( 0, w, 0, h, 0, 1 ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
}

static void end2d ( void )
{
  glMatrixMode   ( GL_PROJECTION ) ;
  glPopMatrix    () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPopMatrix    () ;
  
  glDisable      ( GL_ALPHA_TEST ) ;
  glDisable      ( GL_BLEND ) ;
  glAlphaFunc    ( GL_ALWAYS, 0.0 ) ;
  glBlendFunc    ( GL_ONE, GL_ZERO ) ;
}


static void count_anim_frames( ssgEntity *e )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;
    
    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      count_anim_frames ( br -> getKid ( i ) ) ;
    
    if ( e -> isAKindOf ( ssgTypeSelector() ) )
    {
      ssgSelector* p = (ssgSelector*) e ;
      int num = p -> getMaxKids () ;
      if ( num > num_anim_frames )
        num_anim_frames = num ;
    }
    else if ( e -> isAKindOf ( ssgTypeTransform() ) )
    {
      ssgBase* data = e -> getUserData () ;
      if ( data != NULL && data -> isAKindOf ( ssgTypeTransformArray() ) )
      {
        ssgTransformArray* ta = (ssgTransformArray*) data ;
        int num = ta -> getNum () ;
        if ( num > num_anim_frames )
          num_anim_frames = num ;
      }
    }
  }
  else if ( e -> isAKindOf ( ssgTypeLeaf() ) )
  {
    ssgLeaf* leaf = (ssgLeaf *) e ;
    ssgState* st = leaf -> getState () ;

    if ( st && st -> isAKindOf ( ssgTypeStateSelector() ) )
    {
      ssgStateSelector* ss = (ssgStateSelector*) st ;

      int num = ss -> getNumSteps () ;
      if ( num > num_anim_frames )
        num_anim_frames = num ;
    }
  }
}



// You need to have a object with animation loaded into memory for the following function to make sense.
// Animation can be discreet (in steps) or "smooth" (fluid).
// If you loaded an animation made of several meshes, it will use a selector and therefore always be "discreet"
// If you loaded some transformation matrices like some rotations to rotate the flap of a plane,
// the define DISCREET_STEPS will determine whether the animation is smooth (new code, WK) or 
// in steps ("old" code, Dave McClurg). 
//
// The anim_frame is for discreet animation and an index determining which matrix / which selector will be used.
//
// typically, you will call set_anim_frame directly before you call ssgCullAndDraw 
static void set_anim_frame( ssgEntity *e, int anim_frame )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;
    
    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      set_anim_frame ( br -> getKid ( i ), anim_frame ) ;
    
    if ( e -> isAKindOf ( ssgTypeSelector() ) )
    {
      ssgSelector* p = (ssgSelector*) e ;
      int num = p -> getMaxKids () ;
      if ( num > 0 )
      {
        int frame = anim_frame ;
        frame %= num ;
        p -> selectStep ( frame ) ;
      }
    }
  }
  else if ( e -> isAKindOf ( ssgTypeLeaf() ) )
  {
    ssgLeaf* leaf = (ssgLeaf *) e ;
    ssgState* st = leaf -> getState () ;

    if ( st && st -> isAKindOf ( ssgTypeStateSelector() ) )
    {
      ssgStateSelector* ss = (ssgStateSelector*) st ;
      int num = ss -> getNumSteps () ;
      if ( num > 0 )
      {
        int frame = anim_frame ;
        frame %= num ;
        ss -> selectStep ( frame ) ;
      }
    }
  }
}



static int wire_draw ( ssgEntity* e )
{
  if ( e -> isAKindOf ( ssgTypeLeaf() ) )
  {
    ssgLeaf* leaf = (ssgLeaf*) e ;
    leaf -> drawHighlight ( wire_colour ) ;
  }
  return 1 ;
}


static void wire_update ( ssgEntity *e, int flag )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;
    
    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      wire_update ( br -> getKid ( i ), flag ) ;
  }
  else if ( e -> isAKindOf ( ssgTypeLeaf() ) )
  {
    ssgLeaf* leaf = (ssgLeaf *) e ;
    leaf -> setCallback ( SSG_CALLBACK_POSTDRAW, flag? wire_draw: NULL ) ;
  }
}


static void make_matrix( sgMat4 mat )
{
  SGfloat angle = -EyeAz * SG_DEGREES_TO_RADIANS ;
  sgVec3 eye ;
  eye[0] = (SGfloat) cos (angle) * EyeDist + Ex ;
  eye[1] = (SGfloat) sin (angle) * EyeDist + Ey ;
  angle = EyeEl * SG_DEGREES_TO_RADIANS ;
  eye[2] = (SGfloat) sin (angle) * EyeDist + Ez ;

  sgVec3 center ;
  sgSetVec3 ( center, Ex, Ey, Ez ) ;

  sgVec3 up ;
  sgSetVec3 ( up, 0.0f, 0.0f, 0.1f ) ;

  sgMakeLookAtMat4 ( mat, eye, center, up ) ;
}


static bool get_camera_dir ( ssgEntity* e, sgVec3 eye, sgVec3 target )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;
    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
    {
      if ( get_camera_dir ( br -> getKid ( i ), eye, target ) )
        return true ;
    }
  }
  else if ( e -> isAKindOf ( ssgTypeLeaf() ) )
  {
    ssgLeaf* leaf = (ssgLeaf *) e ;
    if ( leaf -> getNumVertices () >= 2 )
    {
      sgCopyVec3 ( eye, leaf -> getVertex ( 0 ) ) ;
      sgCopyVec3 ( target, leaf -> getVertex ( 1 ) ) ;
      return true ;
    }
  }
  return false ;
}


static void follow_camera ( sgMat4 mat )
{
  if ( camera_object != NULL )
  {
    sgVec3 v1, v2 ;
    if ( camera_object -> isAKindOf ( ssgTypeTransform() ) &&
      get_camera_dir ( camera_object, v1, v2 ) )
    {
      ssgTransform* tr = (ssgTransform*) camera_object ;
      sgMat4 m ;
      tr -> getTransform ( m ) ;

      sgVec3 up = { 0.0f, 0.0f, 1.0f } ;
      sgVec3 eye, target ;
      sgXformPnt3 ( eye, v1, m ) ;
      sgXformPnt3 ( target, v2, m ) ;
      sgMakeLookAtMat4 ( mat, eye, target, up ) ;
    }
  }
}


/*
  The GLUT window reshape event
  Fixed to properly maintain aspect ratio.
*/

static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;

  float aspect = w / (float)h;
  float angle = 0.5 * FOVY * M_PI / 180.0;
  float y = NEAR * tan(angle);
  float x = aspect * y;

  context->setFrustum(-x,x,-y,y,NEAR,FAR);
}



/*
  The GLUT display event
*/

static void display(void)
{
  glClearColor ( 0.2f, 0.7f, 1.0f, 1.0f ) ;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  sgMat4 mat ;
  make_matrix ( mat ) ;
  follow_camera ( mat ) ;

  context->setCamera ( mat );
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable( GL_DEPTH_TEST ) ;
  
  /*
  update virtual time, from "real" time and speed.
  */
  static int last_time = 0;
  int curr_time = glutGet((GLenum)GLUT_ELAPSED_TIME);
  if (last_time != 0)
    // calculate the "virtual time"  
    _ssgGlobTime += 0.001f*(curr_time-last_time)*speed_tab[ speed_index ];

  last_time = curr_time;
  
  //set_anim_frame ( scene, _ssgGlobTime); // currently not needed by our sample data 
 
  /*
  figure out how fast this bus is going
  */
  static int last_time2 = 0;
  if (curr_time >= last_time2 + 2000) {
    fps = frame_counter * 1000.0f / (curr_time - last_time2);
    frame_counter = 0;
    last_time2 = curr_time;
  }
  
  ssgCullAndDraw ( scene ) ;

  /*
  do the interface stuff
  */

  int h = getWindowHeight () ;
  
  begin2d () ;
  
  text -> setFont      ( font ) ;
  text -> setPointSize ( 12 ) ;
  
  if ( scene -> getNumKids() == 0 )
  {
    text -> begin () ;
    glColor3f ( 0.0f, 0.0f, 0.0f ) ;
    text -> start2f ( 4.0f, (float)(h-16) ) ;
    text -> puts ( "controls\n" );
    text -> puts ( " <esc> : exit\n" ) ;
    text -> puts ( " c : clear\n" ) ;
    text -> puts ( " l : load a shape or sequence\n" ) ;
    text -> puts ( " w : toggle wireframe\n" ) ;
    text -> puts ( " <key.pad.plus> : faster animation\n" ) ;
    text -> puts ( " <key.pad.minus> : slower animation\n" ) ;
    text -> puts ( " <left.mouse.button> : rotate\n" ) ;
    text -> puts ( " <right.mouse.button> : zoom\n" ) ;
    text -> end () ;
  }
  else if ( fps > 0 )
  {
    
    char buffer [ PUSTRING_MAX ] ;
    text -> begin () ;
    glColor3f ( 0.0f, 0.0f, 0.0f ) ;
    text -> start2f ( 4.0f, (float)(h-16) ) ;
    sprintf ( buffer, "fps : %.02f\n", fps ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "dist : %.02f\n", EyeDist ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "anim_speed : %d fps\n", speed_tab[ speed_index ] ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "time : %-4d\n", curr_time ) ;
      text -> puts ( buffer ) ;
    text -> end () ;
    
  }
  
  glColor3f ( 1.0f, 1.0f, 1.0f ) ;
  end2d () ;
  
  frame_counter ++;
  
  puDisplay () ;

  glutPostRedisplay () ;
  glutSwapBuffers () ;
}

/*
* keep them vertical; I think this makes a lot of things easier, 
* but maybe it wouldn't be too hard to adapt things to let you go
* upside down
*/
static int ConstrainEl(void)
{
  if (EyeEl <= -90) {
    EyeEl = -89.99f;
    return 1;
  } else if (EyeEl >= 90) {
    EyeEl = 89.99f;
    return 1;
  }
  return 0;
}

/* What to multiply number of pixels mouse moved by to get rotation amount */
#define EL_SENS   0.5f
#define AZ_SENS   0.5f

static void motion(int x, int y)
{
  puMouse ( x, y ) ;
  
  int deltax = x - downx, deltay = y - downy;
  
  //  if ( scene -> getNumKids() > 0 && !file_selector )
  if ( !file_selector )
    
    switch (downb) {
  case GLUT_LEFT_BUTTON:
    EyeEl  = downEl + EL_SENS * deltay;
    ConstrainEl();
    EyeAz  = downAz + AZ_SENS * deltax;
    dAz    = PREV_DAZ*dAz + CUR_DAZ*(lastAz - EyeAz);
    dEl    = PREV_DEL*dEl + CUR_DEL*(lastEl - EyeEl);
    lastAz = EyeAz;
    lastEl = EyeEl;
    break;
  case GLUT_RIGHT_BUTTON:
    {
      float mult = 0.01f;
      if ( fabs(downDist) > 1.0f )
        mult = float( 0.001 * fabs(downDist) );
      EyeDist = downDist + mult*deltay;
      break;
    }
  case GLUT_MIDDLE_BUTTON:
    {
      float mult = 0.01f;
      if ( fabs(EyeDist) > 1.0f )
        mult = float( 0.0005f * fabs(EyeDist) );
      
      sgMat4 mat ;
      make_matrix( mat ) ;
      
      sgVec3 off;
      sgSetVec3( off, mult*deltax, 0, -mult*deltay );
      sgXformVec3( off, mat );
      
      Ex -= off[0];
      Ey -= off[1];
      Ez -= off[2];
    }
    break;
  }
}

static void mouse(int button, int state, int x, int y)
{
  puMouse ( button, state, x, y ) ;
  
  if (state == GLUT_DOWN && downb == -1) {
    downx = x;
    downy = y;
    downb = button;
    
    //    if ( scene -> getNumKids() > 0 && !file_selector )
    if (!file_selector )
      switch ( button )
    {
    case GLUT_LEFT_BUTTON:
      lastEl = downEl = EyeEl;
      lastAz = downAz = EyeAz;
      AzSpin = ElSpin = dAz = dEl = 0;
      AdjustingAzEl = 1;
      break;
    case GLUT_RIGHT_BUTTON:
      downDist = EyeDist;
      break;
    case GLUT_MIDDLE_BUTTON:
      downEx = Ex;
      downEy = Ey;
      downEz = Ez;
      break;
    }
  } else if (state == GLUT_UP && button == downb) {
    downb = -1;
    
    //    if ( scene -> getNumKids() > 0 && !file_selector )
    if ( !file_selector )
      switch ( button )
    {
    case GLUT_LEFT_BUTTON:
      AzSpin =  -dAz;
      if (AzSpin < MIN_AZSPIN && AzSpin > -MIN_AZSPIN)
        AzSpin = 0;	
      ElSpin = -dEl;
      if (ElSpin < MIN_ELSPIN && ElSpin > -MIN_ELSPIN)
        ElSpin = 0; 
      AdjustingAzEl = 0;
      break;
    }
  }
}

static void special(int key, int x, int y)
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;
  
  switch ( key )
  {
  case GLUT_KEY_LEFT:
  case GLUT_KEY_UP:
  case GLUT_KEY_RIGHT:
  case GLUT_KEY_DOWN:
  case GLUT_KEY_PAGE_UP:
  case GLUT_KEY_PAGE_DOWN:
  case GLUT_KEY_HOME:
  case GLUT_KEY_END:
    break;
  }
}

static void pick_cb ( puObject * )
{
  char* str ;
  file_selector -> getValue ( &str ) ;
  
  char fname [PUSTRING_MAX];
  strcpy ( fname, str ) ;
  
  puDeleteObject ( file_selector ) ;
  file_selector = 0 ;

  ssgEntity *obj = ssgLoad ( (char*)fname ); //"test_Med.ase" ) ;

#if 0
  ssgSaveSSG ( "temp.ssg", obj ) ;
  delete obj ;
  obj = ssgLoadSSG ( "temp.ssg" ) ;
#endif

#if 0
  ssgSaveDXF ( "temp.dxf", obj ) ;
  delete obj ;
  obj = ssgLoadDXF ( "temp.dxf" ) ;
#endif
  
  if ( !obj )
    return;
  
  scene -> addKid ( obj ) ;

  num_anim_frames = 0 ;
  count_anim_frames ( scene ) ;
  wire_update ( scene, wire_flag ) ;
  
  SGfloat radius = scene->getBSphere()->getRadius();
  EyeDist = float( radius * 1.5f / tan( float( FOVY/2 * SG_DEGREES_TO_RADIANS ) ) );
  
  sgSphere sp = *( scene -> getBSphere() ) ;
  if ( sp.isEmpty() )
  {
    Ex = 0.0f ;
    Ey = 0.0f ;
    Ez = 0.0f ;
  }
  else
  {
    Ex = sp.getCenter()[ 0 ] ;
    Ey = sp.getCenter()[ 1 ] ;
    Ez = sp.getCenter()[ 2 ] ;
  }

  /* try to find the animated camera */
  camera_object = scene -> getByName ( "FlyCam" ) ;
}



/*
  The GLUT keyboard event
*/

static void keyboard(unsigned char key, int, int)
{
  puKeyboard ( key, PU_DOWN ) ;
  
  switch ( key )
  {
  case 0x1b:  //escape
    exit(0);
    break;
  case '+':
    if ( speed_tab[ speed_index ] != MAX_SPEED )
      speed_index ++ ;
    break;
  case '-':
    if ( speed_tab[ speed_index ] != 0 )
      speed_index -- ;
    break;
  case ' ':
    Ex = 0;
    Ey = 0;
    Ez = 0;
    break;
  case 'l':
    if ( !file_selector )
    {
      file_selector = new puFileSelector ( ( 640 - 320 ) / 2, ( 480 - 270 ) / 2, 320, 270, ARROWS_USED, "data" ) ;
      file_selector -> setCallback ( pick_cb ) ;
    }
    break;
  case 'w':
    wire_flag = !wire_flag ;
    wire_update ( scene, wire_flag ) ;
    break ;
  case 'c':
    //scene -> removeKid ( obj );
    scene -> removeAllKids () ;
    break ;
  }
}

static void idle(void)
{
  EyeEl += ElSpin;
  EyeAz += AzSpin;
  
  /*
  * weird spin thing to make things look
  * look better when you are kept from going
  * upside down while spinning - Isn't great
  */
  if (ConstrainEl()) {
    ElSpin = -ElSpin;
    if (fabs(ElSpin) > fabs(AzSpin))
      AzSpin = float( fabs(ElSpin) * ((AzSpin > 0) ? 1 : -1) );
  }
  
  if (AdjustingAzEl) {
    dAz *= SLOW_DAZ;
    dEl *= SLOW_DEL;
  }
}

static void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;
  fake_argv[0] = "Viewer" ;
  fake_argv[1] = "Simple Scene Graph : Viewer Program." ;
  fake_argv[2] = NULL ;

  /*
    Initialise GLUT
  */

  glutInit              ( &fake_argc, fake_argv ) ;
  glutInitWindowSize    ( 640, 480 ) ;
  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow      ( fake_argv[1] ) ;
  glutDisplayFunc       ( display  ) ;
  glutReshapeFunc       ( reshape  ) ;
  glutKeyboardFunc      ( keyboard ) ;
  glutMouseFunc         ( mouse    ) ;
  glutMotionFunc        ( motion   ) ;
  glutSpecialFunc       ( special  ) ;
  glutIdleFunc          ( idle     ) ;
  
  /*
    Some basic OpenGL setup
  */

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  
  /*
  Initialise PLIB
  */
  
  ssgInit () ;
  puInit () ;
  
  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 200.0f, -500.0f, 500.0f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;


  
  /*
  Set up the path to the data files
  */
  
  ssgModelPath   ( "." ) ;
  ssgTexturePath ( "." ) ;
  
  /*
  Create a root node
  */
  
  scene = new ssgRoot      ;
  context = new ssgContext ;
  context->setCullface( true ) ;
  context->makeCurrent ( ) ;
  
  /*
  ** Set up PU
  */
  text = new fntRenderer () ;
  font = new fntTexFont ( "data/default.txf" ) ;
  
  puFont font1 ( font, 12 ) ;
  puFont font2 ( font, 15 ) ;
  puSetDefaultFonts        ( font1, font2 ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.7f, 0.7f, 0.7f, 1.0f ) ;
} 


int main(int argc, char** argv)
{
  init_graphics ();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}


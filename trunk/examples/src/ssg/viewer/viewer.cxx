/****
* NAME
*   viewer - SSG model viewer
*
* DESCRIPTION
*   using a PUI interface, allow user to load a model,
*   animate the model, and manipulate the camera.
*   several models can be loaded at the same time.
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   September 6, 2000
*
* MODIFICATION HISTORY
****/

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

#include <GL/glut.h>

#include <plib/ssg.h>
#include <plib/fnt.h>
#include <plib/pu.h>

#define ARROWS_USED 1
/*
!!!FIXME!!!
Defines what type of arrows to uses with puFilePicker
0 - No arrows
1 - Single move arrows
2 - Single move and Jump 10 moves
*/

/*
scene graph
*/
ssgRoot *scene = NULL ;

/*
font vars
*/
fntRenderer *text ;
fntTexFont *font ;

puFilePicker* file_picker = 0 ;

/*
frame rate vars
*/
int frame_counter = 0;
float fps = 0;

/*
animation vars
*/
#define MAX_SPEED 60
static const int speed_tab [] =
{ 0, 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60 };

int speed_index = 0 ;
int anim_delay = -1 ;
int anim_frame ;
int num_anim_frames ;

/*
spinner vars
*/
int downx, downy;   /* for tracking mouse position */
int downb = -1;     /* and button status */

GLfloat downDist, downEl, downAz, /* for saving state of things */
downEx, downEy, downEz;   /* when button is pressed */

GLfloat dAz, dEl, lastAz, lastEl;  /* to calculate spinning w/ polar motion */
GLfloat AzSpin = 0.0f, ElSpin = 0.0f;
int     AdjustingAzEl = 0;

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
GLfloat EyeDist= 100.0f;
GLfloat EyeAz  = 0.0f;
GLfloat EyeEl  = 30.0f;
GLfloat Ex     = 0.0f;
GLfloat Ey     = 0.0f;
GLfloat Ez     = 0.0f;

#define FOV 60.0f

int getWindowHeight () { return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ; }
int getWindowWidth  () { return glutGet ( (GLenum) GLUT_WINDOW_WIDTH  ) ; }

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
  glAlphaFunc    ( GL_GREATER, 0.1 ) ;
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


void count_anim_frames( ssgEntity *e )
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


void set_anim_frame( ssgEntity *e )
{
  if ( e -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) e ;
    
    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      set_anim_frame ( br -> getKid ( i ) ) ;
    
    if ( e -> isAKindOf ( ssgTypeSelector() ) )
    {
      ssgSelector* p = (ssgSelector*) e ;
      int num = p -> getMaxKids () ;
      if ( num > 0 )
      {
        int frame = anim_frame ;
        if ( frame >= num )
          frame = num-1 ;
        p -> selectStep ( frame ) ;
      }
    }
    else if ( e -> isAKindOf ( ssgTypeTransform() ) )
    {
      ssgBase* data = e -> getUserData () ;
      if ( data != NULL && data -> isAKindOf ( ssgTypeTransformArray() ) )
      {
        ssgTransform* p = (ssgTransform*) e ;
        ssgTransformArray* ta = (ssgTransformArray*) data ;
        int num = ta -> getNum () ;
        if ( num > 0 )
        {
          int frame = anim_frame ;
          if ( frame >= num )
            frame = num-1 ;
          ta -> selection = frame ;
          p -> setTransform ( *( ta -> get ( ta -> selection ) ) ) ;
        }
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
        if ( frame >= num )
          frame = num-1 ;
        ss -> selectStep ( frame ) ;
      }
    }
  }
}


void make_matrix( sgMat4 mat )
{
  SGfloat angle = -EyeAz * SG_DEGREES_TO_RADIANS ;
  sgVec3 eye ;
  eye[0] = (SGfloat) cos (angle) * EyeDist ;
  eye[1] = (SGfloat) sin (angle) * EyeDist ;
  angle = EyeEl * SG_DEGREES_TO_RADIANS ;
  eye[2] = (SGfloat) sin (angle) * EyeDist ;
  
  sgVec3 center ;
  sgSetVec3 ( center, Ex, Ey, Ez ) ;
  
  sgVec3 up ;
  sgSetVec3 ( up, 0.0f, 0.0f, 0.1f ) ;
  
  sgMakeLookAtMat4 ( mat, eye, center, up ) ;
}


/*
  The GLUT window reshape event
*/

void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}


/*
  The GLUT display event
*/

void display(void)
{
  glClearColor ( 0.2f, 0.7f, 1.0f, 1.0f ) ;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  sgMat4 mat ;
  make_matrix ( mat ) ;
  ssgSetCamera ( mat );
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable( GL_DEPTH_TEST ) ;
  
  /*
  increment the frame counter
  */
  static int last_time = 0;
  int curr_time = glutGet((GLenum)GLUT_ELAPSED_TIME);
  while (curr_time >= last_time + 1000/60)
  {
    last_time += 1000/60;

    static int anim_time = 0 ;
    if ( anim_delay != -1 && ++ anim_time >= anim_delay )
    {
      if ( ++ anim_frame >= num_anim_frames )
        anim_frame = 0 ;
      set_anim_frame ( scene ) ;
      anim_time = 0 ;
    }
  }

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
    glColor3f ( 0, 0, 0 ) ;
    text -> start2f ( (float)4, (float)(h-16) ) ;
    text -> puts ( "controls\n" );
    text -> puts ( " <esc> : exit\n" ) ;
    text -> puts ( " c : clear\n" ) ;
    text -> puts ( " l : load a shape or sequence\n" ) ;
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
    glColor3f ( 0, 0, 0 ) ;
    text -> start2f ( (float)4, (float)(h-16) ) ;
    sprintf ( buffer, "fps : %.02f\n", fps ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "dist : %.02f\n", EyeDist ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "anim_speed : %d fps\n", speed_tab[ speed_index ] ) ;
      text -> puts ( buffer ) ;
    sprintf ( buffer, "time : %-4d, %d\n", curr_time, anim_frame ) ;
      text -> puts ( buffer ) ;
    text -> end () ;
  }
  
  glColor3f ( 1, 1, 1 ) ;
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
int ConstrainEl(void)
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

void motion(int x, int y)
{
  puMouse ( x, y ) ;
  
  int deltax = x - downx, deltay = y - downy;
  
  //  if ( scene -> getNumKids() > 0 && !file_picker )
  if ( !file_picker )
    
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

void mouse(int button, int state, int x, int y)
{
  puMouse ( button, state, x, y ) ;
  
  if (state == GLUT_DOWN && downb == -1) {
    downx = x;
    downy = y;
    downb = button;
    
    //    if ( scene -> getNumKids() > 0 && !file_picker )
    if (!file_picker )
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
    
    //    if ( scene -> getNumKids() > 0 && !file_picker )
    if ( !file_picker )
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

void special(int key, int x, int y)
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

void pick_cb ( puObject * )
{
  char* str ;
  file_picker -> getValue ( &str ) ;
  
  char fname [PUSTRING_MAX];
  strcpy ( fname, str ) ;
  
  puDeleteObject ( file_picker ) ;
  file_picker = 0 ;

  ssgEntity *obj = ssgLoad ( (char*)fname ); //"test_Med.ase" ) ;

#if 0
  ssgSaveSSG ( "temp.ssg", obj ) ;
  delete obj ;
  obj = ssgLoadSSG ( "temp.ssg" ) ;
#endif

#if 0
  ssgSaveASE ( "data/temp.ase", obj ) ;
  delete obj ;
  obj = ssgLoadASE ( "temp.ase" ) ;
#endif
  
  if ( !obj )
    return;
  
  scene -> addKid ( obj ) ;

  num_anim_frames = 0 ;
  count_anim_frames ( scene ) ;
  
  SGfloat radius = scene->getBSphere()->getRadius();
  EyeDist = float( radius * 1.5f / tan( float( FOV/2 * SG_DEGREES_TO_RADIANS ) ) );
  
  sgSphere sp = *( scene -> getBSphere() ) ;
  Ex = - sp.getCenter()[ 0 ] ;
  Ey = - sp.getCenter()[ 1 ] ;
  Ez = - sp.getCenter()[ 2 ] ;
}


int get_delay( int speed_index )
{
  int fps = speed_tab[ speed_index ] ;
  if ( fps > 0 )
    return MAX_SPEED / fps ;
  return -1 ;
}


/*
  The GLUT keyboard event
*/

void keyboard(unsigned char key, int, int)
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
    anim_delay = get_delay( speed_index );
    break;
  case '-':
    if ( speed_tab[ speed_index ] != 0 )
      speed_index -- ;
    anim_delay = get_delay( speed_index );
    break;
  case ' ':
    Ex = 0;
    Ey = 0;
    Ez = 0;
    break;
  case 'l':
    if ( !file_picker )
    {
      file_picker = new puFilePicker ( ( 640 - 320 ) / 2, ( 480 - 270 ) / 2, 320, 270, ARROWS_USED, "data" ) ;
      file_picker -> setCallback ( pick_cb ) ;
    }
    break;
  case 'c':
    //scene -> removeKid ( obj );
    scene -> removeAllKids () ;
    break ;
  }
}

void idle(void)
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

void init_graphics ()
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
  Set up the viewing parameters
  */
  
  ssgSetFOV     ( FOV, 0.0f ) ;
  ssgSetNearFar ( 2.0f, 10000.0f ) ;
  
  /*
    Set up the Sun.
  */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 200.0f, -500.0f, 500.0f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  
  /*
  Set up the path to the data files
  */
  
  ssgModelPath   ( "data" ) ;
  ssgTexturePath ( "data" ) ;
  
  /*
  Create a root node
  */
  
  scene = new ssgRoot      ;
  
  /*
  ** Set up PU
  */
  text = new fntRenderer () ;
  font = new fntTexFont ( "data/sorority.txf" ) ;
  
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


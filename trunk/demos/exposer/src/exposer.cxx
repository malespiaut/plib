#include "exposer.h"

int curr_button = 0 ;
int scroll_controllers = 0 ;

EventList * eventList = NULL ;
TimeBox   *   timebox = NULL ;
Floor     *    ground = NULL ;
ssgRoot   * skinScene = NULL ;
ssgRoot   * boneScene = NULL ;
ssgRoot   *sceneScene = NULL ;

puText     *message     ;
puButton   *hideBones   ;
puButton   *hideSkin    ;
puButton   *hideGround  ;
puButton   *hideScene   ;
puSlider   *scroller    ;
puMenuBar  *menuBar     ;

puSlider   *rangeSlider ;
puDial     *  panSlider ;
puDial     * tiltSlider ;

puInput    *show_angle  ;

void setShowAngle ( float a )
{
  show_angle -> setValue ( a ) ;
  show_angle -> rejectInput () ;
}


static void update_eye_motion ()
{
  float r ; rangeSlider -> getValue ( & r ) ; r *=  30.0f ; r +=   2.0f ;
  float h ;   panSlider -> getValue ( & h ) ; h *= 360.0f ; h += 180.0f ;
  float p ;  tiltSlider -> getValue ( & p ) ; p *= 360.0f ; p += 180.0f ;

  sgMat4 mat ;
  sgVec3 vec ;

  sgMakeCoordMat4 ( mat, 0, 0, 0, h, p, 0 ) ;
  sgSetVec3       ( vec, 0, r, 0 ) ;
  sgXformPnt3 ( vec, mat ) ;
  sgNegateVec3 ( vec ) ;
  sgCopyVec3 ( mat[3], vec ) ;
  ssgSetCamera ( mat ) ;
  transformModel ( boneScene, timebox->getCursorTime () ) ;
}


static void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
  ssgSetFOV  ( 60.0f, 60.0f * (float) h / (float) w ) ;
}



static void specialfn ( int key, int, int )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;
}
 

static void keyfn ( unsigned char key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;
}
 

static void passmotionfn ( int x, int y )
{
  puMouse ( x, y ) ;
}
 

static void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
  timebox->updateEventQueue ( curr_button, x, y, FALSE ) ;
}
 

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;

  curr_button = button ;
  timebox->updateEventQueue ( curr_button, x, y, updown == PU_DOWN ) ;
}


static void scrollerCB ( puObject *ob )
{
  scroll_controllers = (int)(((float)getNumBones()) * ob -> getFloatValue ()) ;
}



static void exitCB ( puObject *ob )
{
  if ( ob -> getValue () )
    exit ( 1 ) ;
}



/*
  CALLBACK FUNCTIONS FOR GUI.
*/
 
static void deleteEventCB   ( puObject * ) { timebox -> deleteEvent   () ; }
static void addNewEventCB   ( puObject * ) { timebox -> addNewEvent   () ; }
static void reverseRegionCB ( puObject * ) { timebox -> reverseRegion () ; }
static void deleteAllCB     ( puObject * ) { timebox -> deleteAll     () ; }
static void deleteRegionCB  ( puObject * ) { timebox -> deleteRegion  () ; }
static void deleteRegionAndCompressCB ( puObject * )
                                    { timebox -> deleteRegionAndCompress () ; }

static void zoom_nrm_CB     ( puObject * ) { timebox -> setZoom ( 1.0f ) ; }
static void zoom_in_CB      ( puObject * ) { timebox -> setZoom (
                                              timebox -> getZoom () * 1.5 ) ; }
static void zoom_out_CB     ( puObject * ) { float scale = timebox->getZoom() / 1.5 ;
                    timebox -> setZoom ( ( scale <= 1.0f ) ? 1.0f : scale ) ; }

static void add_1_CB ( puObject * ) {timebox->setMaxTime(timebox->getMaxTime()+1.0f);}
static void add_2_CB ( puObject * ) {timebox->setMaxTime(timebox->getMaxTime()+2.0f);}
static void add_5_CB ( puObject * ) {timebox->setMaxTime(timebox->getMaxTime()+5.0f);}

void opaqueBones ()
{
  if ( getBoneState() != NULL )
    getBoneState() -> disable ( GL_BLEND ) ;
}
 
 
void blendBones ()
{
  if ( getBoneState() != NULL )
    getBoneState() -> enable ( GL_BLEND ) ;
}


/* GLUT callbacks */

static void redraw ()
{
  int i ;

  timebox->updateVCR () ;

  update_eye_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  glEnable ( GL_DEPTH_TEST ) ;

  if ( ! hideScene -> getValue () && sceneScene != NULL )
    ssgCullAndDraw ( sceneScene ) ;

  if ( ! hideGround -> getValue () )
    ground -> draw () ;

  if ( ! hideSkin -> getValue () )
  {
    ssgCullAndDraw ( skinScene ) ;
    blendBones () ;
  }
  else
    opaqueBones () ;

  if ( ! hideBones -> getValue () )
  {
    glClear ( GL_DEPTH_BUFFER_BIT ) ;
    ssgCullAndDraw ( boneScene ) ;
  }

  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  glAlphaFunc ( GL_GREATER, 0.1f ) ;
  glEnable    ( GL_BLEND ) ;
  glDisable   ( GL_DEPTH_TEST ) ;

  for ( i = 0 ; i < getNumBones() ; i++ )
    getBone ( i ) -> widget -> hide () ;

  for ( i = 0 ; i < 10 && i+scroll_controllers < getNumBones() ; i++ )
  {
    getBone ( i + scroll_controllers ) -> widget -> setPosition ( 24, 70+i*40);

    if ( eventList -> getCurrentEvent() == NULL )
    {
      getBone ( i + scroll_controllers ) -> widget -> reveal  () ;
      getBone ( i + scroll_controllers ) -> widget -> greyOut () ;
    }
    else
    {
      getBone ( i + scroll_controllers ) -> widget -> reveal   () ;
      getBone ( i + scroll_controllers ) -> widget -> activate () ;
    }
  }

  puDisplay () ;

  timebox -> draw () ;

  glutPostRedisplay () ;
  glutSwapBuffers   () ;
}


/* Menu bar entries: */
 
char      *file_submenu    [] = {  "Exit", 
                                   "------------", 
                                   "Save Bones As...",
                                   "Load Bones",
                                   "Load Scenery",
                                   "Load Model",
                                   NULL } ;

puCallback file_submenu_cb [] = { exitCB,
                                  NULL,
                                  bnsaveCB,
                                  bnloadCB,
                                  scloadCB,
                                  loadCB,
                                  NULL } ;

char      *view_submenu    [] = { "Zoom Timeline In", 
                                  "Zoom Timeline Out",
                                  "Zoom Timeline 1:1",
                                  NULL } ;

puCallback view_submenu_cb [] = { zoom_in_CB,
				  zoom_out_CB,
				  zoom_nrm_CB,
                                  NULL } ;

char      *time_submenu    [] = { "Add 5 seconds", 
                                  "Add 2 seconds",
                                  "Add 1 second",
                                  "------------", 
                                  "Reverse region",
                                  "Delete region & compress",
                                  "Delete region",
                                  "Delete selected event",
                                  "Add new event",
                                  NULL } ;

puCallback time_submenu_cb [] = { add_5_CB,
                                  add_2_CB,
                                  add_1_CB,
                                  NULL,
                                  reverseRegionCB,
                                  deleteRegionAndCompressCB,
                                  deleteRegionCB,
                                  deleteEventCB,
                                  addNewEventCB,
                                  NULL } ;


static void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;

  fake_argv[0] = "ExPoser" ;
  fake_argv[1] = "ExPoser - Skin and Bones animation for PLIB." ;
  fake_argv[2] = NULL ;

  /* Initialise GLUT */

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 800, 600 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
  glutDisplayFunc        ( redraw    ) ;
  glutReshapeFunc        ( reshape   ) ;
  glutKeyboardFunc       ( keyfn     ) ;
  glutSpecialFunc        ( specialfn ) ;
  glutMouseFunc          ( mousefn   ) ;
  glutMotionFunc         ( motionfn  ) ;
  glutPassiveMotionFunc  ( passmotionfn  ) ;
 
  /* Initialise SSG & PUI */

  ssgInit () ;
  puInit  () ;
  puSetDefaultStyle ( PUSTYLE_SMALL_SHADED ) ;                           
  puSetDefaultFonts ( PUFONT_HELVETICA_10, PUFONT_HELVETICA_10 ) ;

  /* Some basic OpenGL setup */

  glClearColor ( 0.2f, 0.5f, 0.2f, 1.0f ) ;
  ssgSetFOV     ( 60.0f, 0.0f ) ;
  ssgSetNearFar ( 0.1f, 700.0f ) ;

  /* Set up the Sun. */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, 0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;

  /* GUI setup. */

  timebox = new TimeBox ( eventList ) ;
  ground  = new Floor   () ;

  menuBar = new puMenuBar () ;
  {
    menuBar -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
    menuBar -> add_submenu ( "View", view_submenu, view_submenu_cb ) ;
    menuBar -> add_submenu ( "Time", time_submenu, time_submenu_cb ) ;
  }
  menuBar -> close () ;

  show_angle   =  new puInput ( 5, 560, 80, 580 ) ;
  show_angle   -> setValue ( "" ) ;
  show_angle   -> rejectInput () ;
  message      =  new puText    ( 80, 560 ) ;
  message      -> setLabel      ( "Angle"  ) ;

  scroller     =  new puSlider  ( 5, 70, 400, TRUE ) ;
  scroller     -> setCBMode     ( PUSLIDER_DELTA   ) ;
  scroller     -> setDelta      ( 0.01             ) ;
  scroller     -> setCallback   ( scrollerCB       ) ;

    panSlider  =  new puDial    ( 0, 0, 40 ) ;
    panSlider  -> setCBMode     ( PUSLIDER_DELTA ) ;
    panSlider  -> setDelta      ( 0.01f   ) ;
    panSlider  -> setValue      ( 0.5f    ) ;
  message      =  new puText    ( 0, 40 ) ;
  message      -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message      -> setLabel      ( "Pan"  ) ;

   tiltSlider  =  new puDial    ( 40, 0, 40 ) ;
   tiltSlider  -> setCBMode     ( PUSLIDER_DELTA ) ;
   tiltSlider  -> setDelta      ( 0.01f   ) ;
   tiltSlider  -> setValue      ( 0.5f    ) ;
  message      =  new puText    ( 40, 40 ) ;
  message      -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message      -> setLabel      ( "Tilt"  ) ;

  hideBones    =  new puButton  ( 80, 0, "Bones" ) ;
  hideBones    -> setValue      ( TRUE ) ;
  hideSkin     =  new puButton  ( 119, 0, "Skin" ) ;
  hideSkin     -> setValue      ( FALSE ) ;
  hideGround   =  new puButton  ( 149, 0, "Floor" ) ;
  hideGround   -> setValue      ( FALSE ) ;
  hideScene    =  new puButton  ( 183, 0, "Scene" ) ;
  hideScene    -> setValue      ( FALSE ) ;

  rangeSlider  =  new puSlider  ( 80, 20, 141, FALSE ) ;
  rangeSlider  -> setCBMode     ( PUSLIDER_DELTA ) ;
  rangeSlider  -> setDelta      ( 0.01    ) ;
  message      =  new puText    ( 80, 40 ) ;
  message      -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message      -> setLabel      ( "Zoom"  ) ;
}


static void init_database ()
{
  skinScene = new ssgRoot ;
  boneScene = new ssgRoot ;
}


static void help ()
{
  fprintf ( stderr, "\n\n" ) ;
  fprintf ( stderr, "exposer: Usage -\n\n" ) ;
  fprintf ( stderr, "    exposer\n" ) ;
  fprintf ( stderr, "\n\n" ) ;
}


int main ( int argc, char **argv )
{
  eventList = new EventList ;

  init_graphics     () ;
  init_database     () ;
  init_bones        () ;

  loadCB ( NULL ) ;

  glutPostRedisplay () ;
  glutMainLoop      () ;
  return 0 ;
}




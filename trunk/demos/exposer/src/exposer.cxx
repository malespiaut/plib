#include "exposer.h"

#define ARROWS_USED 1

int   curr_button = 0 ;

int scroll_controllers = 0 ;

unsigned int floor_texhandle =  0 ;
       float floor_z_coord   = -1 ;

puFileSelector *file_selector = NULL ;
puButton *dialog_button  = NULL ;

ssgRoot * skinScene = NULL ;
ssgRoot * boneScene = NULL ;
ssgRoot *sceneScene = NULL ;


char lastModelFilePath [ PUSTRING_MAX ] ;
char lastModelFileName [ PUSTRING_MAX ] ;

void loadCB ( puObject * ) ;
void scloadCB ( puObject * ) ;
void bnloadCB ( puObject * ) ;

puInput    *show_angle  ;
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


unsigned char floorTexture0 [] =
{
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  0, 0, 0, 0, 255, 255, 255, 255,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0,
  255, 255, 255, 255, 0, 0, 0, 0
} ;

unsigned char floorTexture1 [] =
{
  0, 0, 255, 255,
  0, 0, 255, 255,
  255, 255, 0, 0,
  255, 255, 0, 0
} ;

unsigned char floorTexture2 [] =
{
  0, 255,
  255, 0
} ;

unsigned char floorTexture3 [] =
{
  127
} ;



void setShowAngle ( float a )
{
  show_angle -> setValue ( a ) ;
  show_angle -> rejectInput () ;
}


void update_motion ()
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
  transformModel ( boneScene, getCursor () ) ;
}


void reshape ( int w, int h )
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
  updateEventQueue ( curr_button, x, y, FALSE ) ;
}
 

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;

  curr_button = button ;
  updateEventQueue ( curr_button, x, y, updown == PU_DOWN ) ;
}



void dismissDialogCB ( puObject * )
{
  dialog_button  -> hide () ;
}

void dialog ( char *msg, float r, float g, float b )
{
  dialog_button -> setLegend ( msg ) ;
  dialog_button -> setColorScheme ( r, g, b, 1 ) ;
  dialog_button -> reveal () ;
}

void bnsavepickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;

  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;

    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  char *p = NULL ;
  int i ;

  for ( i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  /* SAVE THE BONES */

  if ( file_selector->getStringValue()[0] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  FILE *fd = fopen ( file_selector->getStringValue(), "wa" ) ;

  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( fd == NULL )
  {
    dialog ( "FAILED TO SAVE BONES!", 1, 0, 0 ) ;
    return ;
  }

  fprintf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
                        getNumBones(), getNumEvents(), getMaxTime (),
                        -floor_z_coord, getVCRGroundSpeed() ) ;

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> write ( fd ) ;

  for ( i = 0 ; i < getNumEvents () ; i++ )
    getEvent ( i ) -> write ( fd ) ;

  fclose ( fd ) ;
  dialog ( "BONES WERE SAVED OK.", 1, 1, 0 ) ;
}



void bnpickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    return ;
  }

  char *p = NULL ;
  int i ;

  for ( i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  /* LOAD THE BONES */

  if ( file_selector->getStringValue()[0] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    return ;
  }

  FILE *fd = fopen ( file_selector->getStringValue(), "ra" ) ;

  if ( fd == NULL )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = NULL ;
    return ;
  }

  deleteAll () ;

  int numbones, numevents ;
  float tmp_floor_z_coord, maxtime, new_ground_speed ;

  /* Don't use the floor_z_coord from the file. */

  fscanf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
                &numbones, &numevents,
                &maxtime, &tmp_floor_z_coord, &new_ground_speed ) ;

  setMaxTime ( maxtime ) ;
  setVCRGroundSpeed ( new_ground_speed ) ;

  if ( numbones != getNumBones () )
  {
    fprintf ( stderr,
      "Number of bones in model doesn't agree with number in bones file!\n" ) ;
    exit ( 1 ) ;
  }

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> read ( fd ) ;

  for ( i = 0 ; i < numevents ; i++ )
  {
    Event *e = new Event ( numbones, (float) i ) ;
    e -> read ( fd ) ;
    addEvent ( e ) ;
  }

  fclose ( fd ) ;
  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;
}



void scpickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( path [ 0 ] == '\0' )
    return ;

  if ( strlen ( path ) >= 6 && strcmp(&path[strlen(path)-6], ".bones" ) == 0 )
  {
    fprintf ( stderr, "I think you tried to load a BONES file as 3D model.\n");
    fprintf ( stderr, "Try again!\n");
    scloadCB ( NULL ) ;
    return ;
  }

  char *p = NULL ;

  for ( int i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  delete sceneScene ;
  sceneScene = new ssgRoot ;
  sceneScene -> addKid ( ssgLoad ( fname, NULL ) ) ;
}


void pickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  puDeleteObject ( file_selector ) ;
  file_selector = NULL ;

  if ( path [ 0 ] == '\0' )
    return ;

  if ( strlen ( path ) >= 6 && strcmp(&path[strlen(path)-6], ".bones" ) == 0 )
  {
    fprintf ( stderr, "I think you tried to load a BONES file as 3D model.\n");
    fprintf ( stderr, "Try again!\n");
    loadCB ( NULL ) ;
    return ;
  }

  char *p = NULL ;

  for ( int i = strlen(path) ; i >= 0 ; i-- )
    if ( path[i] == '/' || path[i] == '\\' )
    {
      p = & ( path[i+1] ) ;
      path[i] = '\0' ;
      break ;
    }

  if ( p == NULL )
  {
    ssgModelPath   ( "." ) ;
    ssgTexturePath ( "." ) ;
    strcpy ( fname, path ) ;
  }
  else
  {
    ssgModelPath   ( path ) ;
    ssgTexturePath ( path ) ;
    strcpy ( fname, p ) ;
  }

  strcpy ( lastModelFilePath, path ) ;
  strcpy ( lastModelFileName, fname ) ;

  skinScene -> addKid ( ssgLoad ( fname, NULL ) ) ;
  boneScene -> addKid ( extractBones ( skinScene ) ) ;

  extractVertices ( skinScene ) ;
  deleteAll () ;

  Event *e = new Event ( getNumBones(), 0.0f ) ;
  addEvent ( e ) ;
  setCurrentEvent ( e ) ;

  floor_z_coord = getLowestVertexZ () ;
}



void scrollerCB ( puObject *ob )
{
  scroll_controllers = (int)(((float)getNumBones()) * ob -> getFloatValue ()) ;
}


void bnloadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( ( 640 - 320 ) / 2,
			   ( 480 - 270 ) / 2,
			   320, 270, ARROWS_USED, lastModelFilePath,
                             "Load Bones from..." ) ;
    file_selector -> setCallback ( bnpickfn ) ;

    char guess_fname [ PUSTRING_MAX ] ;
    strcpy ( guess_fname, lastModelFileName ) ;

    for ( int i = strlen ( guess_fname ) ; i >= 0 ; i-- )
      if ( guess_fname [ i ] == '.' )
      {
        guess_fname[i] = '\0' ;
        break ;
      }

    strcat ( guess_fname, ".bones" ) ;
    file_selector -> setInitialValue ( guess_fname ) ;
  }

}


void bnsaveCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( ( 640 - 320 ) / 2,
			  ( 480 - 270 ) / 2,
			  320, 270, ARROWS_USED, lastModelFilePath,
                            "Save Bones As..." ) ;
    file_selector -> setCallback ( bnsavepickfn ) ;

    char guess_fname [ PUSTRING_MAX ] ;
    strcpy ( guess_fname, lastModelFileName ) ;

    for ( int i = strlen ( guess_fname ) ; i >= 0 ; i-- )
      if ( guess_fname [ i ] == '.' )
      {
        guess_fname[i] = '\0' ;
        break ;
      }

    strcat ( guess_fname, ".bones" ) ;
    file_selector -> setInitialValue ( guess_fname ) ;
  }
}



void scloadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( ( 640 - 320 ) / 2,
                                 ( 480 - 270 ) / 2,
                                 320, 270, ARROWS_USED, "", "Load Scenery from..." ) ;
    file_selector -> setCallback ( scpickfn ) ;
  }
}


void loadCB ( puObject * )
{
  if ( file_selector == NULL )
  {
    file_selector = new puFileSelector ( ( 640 - 320 ) / 2,
                                 ( 480 - 270 ) / 2,
                                 320, 270, ARROWS_USED, "", "Load from..." ) ;
    file_selector -> setCallback ( pickfn ) ;
  }
}


void exitCB ( puObject *ob )
{
  if ( ob -> getValue () )
    exit ( 1 ) ;
}


void drawFloor ()
{
  glMatrixMode ( GL_PROJECTION ) ;
  _ssgCurrentContext->loadProjectionMatrix () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  _ssgCurrentContext->loadModelviewMatrix () ;

  glDisable ( GL_LIGHTING   ) ;
  glEnable  ( GL_TEXTURE_2D ) ;
  glEnable  ( GL_CULL_FACE  ) ;
  glBindTexture ( GL_TEXTURE_2D, floor_texhandle ) ;
  glColor4f ( 1, 1, 1, 1 ) ;
  glBegin ( GL_QUADS ) ;
  glTexCoord2f ( -30, getVCRGroundPosition() - 30 ) ;
  glVertex3f ( -30, -30, floor_z_coord ) ;
  glTexCoord2f (  30, getVCRGroundPosition() - 30 ) ;
  glVertex3f (  30, -30, floor_z_coord ) ;
  glTexCoord2f (  30, getVCRGroundPosition() + 30 ) ;
  glVertex3f (  30,  30, floor_z_coord ) ;
  glTexCoord2f ( -30, getVCRGroundPosition() + 30 ) ;
  glVertex3f ( -30,  30, floor_z_coord ) ;
  glEnd () ;
  glDisable ( GL_TEXTURE_2D ) ;
  glEnable  ( GL_LIGHTING   ) ;
}


/*
  The GLUT redraw event
*/

void redraw ()
{
  int i ;

  updateVCR () ;
  update_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  glEnable ( GL_DEPTH_TEST ) ;

  if ( ! hideScene -> getValue () && sceneScene != NULL )
  {
    ssgCullAndDraw ( sceneScene ) ;
  }
  else
  if ( ! hideGround -> getValue () )
    drawFloor () ;

  if ( ! hideSkin -> getValue () )
  {
    ssgCullAndDraw ( skinScene ) ;
    blendBones () ;
  }
  else
    opaqueBones () ;

  if ( ! hideBones -> getValue () )
  {
    glClear  ( GL_DEPTH_BUFFER_BIT ) ;

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

    if ( getCurrentEvent() == NULL )
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

  drawTimeBox () ;

  glutPostRedisplay () ;
  glutSwapBuffers   () ;
}


/* Menu bar entries: */
 
char      *file_submenu    [] = {  "Exit", 
                                   "------------", 
                                   "Save Bones As...",
                                   "Load Bones",
                                   "Load Scenery",
                                   "Load Model",  NULL } ;
puCallback file_submenu_cb [] = {  exitCB,
                                   NULL,
                                   bnsaveCB,
                                   bnloadCB,
                                   scloadCB,
                                   loadCB,
                                   NULL } ;


char      *view_submenu    [] = {  "Zoom Timeline In", 
                                   "Zoom Timeline Out",
                                   "Zoom Timeline 1:1",
                                   NULL } ;

puCallback view_submenu_cb [] = {  zoom_in_CB,
				   zoom_out_CB,
				   zoom_nrm_CB,
                                   NULL } ;

char      *time_submenu    [] = {  "Add 5 seconds", 
                                   "Add 2 seconds",
                                   "Add 1 second",
                                   "------------", 
                                   "Reverse region",
                                   "Delete region & compress",
                                   "Delete region",
                                   "Delete selected event",
                                   "Add new event",
                                   NULL } ;
puCallback time_submenu_cb [] = {  add_5_CB,
                                   add_2_CB,
                                   add_1_CB,
                                   NULL,
                                   reverseRegionCB,
                                   deleteRegionAndCompressCB,
                                   deleteRegionCB,
                                   deleteEventCB,
                                   addNewEventCB,
                                   NULL } ;


void init_graphics ()
{
  int   fake_argc = 1 ;
  char *fake_argv[3] ;

  fake_argv[0] = "ExPoser" ;
  fake_argv[1] = "ExPoser - Skin and Bones animation for PLIB." ;
  fake_argv[2] = NULL ;

  /*
    Initialise GLUT
  */

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
 
  /*
    Initialise SSG
  */

  ssgInit () ;
  puInit  () ;
  puSetDefaultStyle ( PUSTYLE_SMALL_SHADED ) ;                           
  puSetDefaultFonts ( PUFONT_HELVETICA_10, PUFONT_HELVETICA_10 ) ;

  /*
    Some basic OpenGL setup
  */

  glClearColor ( 0.2f, 0.5f, 0.2f, 1.0f ) ;

  /*
    Set up the viewing parameters
  */


  ssgSetFOV     ( 60.0f, 0.0f ) ;
  ssgSetNearFar ( 1.0f, 700.0f ) ;

  /* Set up the Sun. */

  sgVec3 sunposn ;
  sgSetVec3 ( sunposn, 0.2f, 0.5f, 0.5f ) ;
  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;

  /* File I/O. */

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

  initTimeBox () ;

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

  glGenTextures   ( 1, & floor_texhandle ) ;
  glBindTexture   ( GL_TEXTURE_2D, floor_texhandle ) ;
  glTexEnvi       ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                            GL_LINEAR_MIPMAP_LINEAR ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
  glTexImage2D  ( GL_TEXTURE_2D, 0, 1, 8, 8,
                  FALSE /* Border */, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                  floorTexture0 ) ;
  glTexImage2D  ( GL_TEXTURE_2D, 1, 1, 4, 4,
                  FALSE /* Border */, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                  floorTexture1 ) ;
  glTexImage2D  ( GL_TEXTURE_2D, 2, 1, 2, 2,
                  FALSE /* Border */, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                  floorTexture2 ) ;
  glTexImage2D  ( GL_TEXTURE_2D, 3, 1, 1, 1,
                  FALSE /* Border */, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                  floorTexture3 ) ;
  glBindTexture ( GL_TEXTURE_2D, 0 ) ;

  dialog_button = new puButton    ( 300, 240, "" ) ;
  dialog_button -> setSize ( 300, 40 ) ;
  dialog_button -> setLegendFont  ( PUFONT_TIMES_ROMAN_24 ) ;
  dialog_button -> setCallback    ( dismissDialogCB ) ;
  dialog_button -> setColorScheme ( 1, 1, 0, 1 ) ;
  dialog_button -> hide () ;
}

void init_database ()
{
  skinScene = new ssgRoot ;
  boneScene = new ssgRoot ;
}



void help ()
{
  fprintf ( stderr, "\n\n" ) ;
  fprintf ( stderr, "exposer: Usage -\n\n" ) ;
  fprintf ( stderr, "    exposer\n" ) ;
  fprintf ( stderr, "\n\n" ) ;
}



int main ( int argc, char **argv )
{
  init_graphics     () ;
  init_database     () ;
  init_bones        () ;
  init_events       () ;
  loadCB      ( NULL ) ;
  glutPostRedisplay () ;
  glutMainLoop      () ;
  return 0 ;
}




#include "exposer.h"

#define ARROWS_USED 1

#define TIMEBOX_INITIAL_DURATION   10.0f   /* Seconds */

float timebox_maxtime = TIMEBOX_INITIAL_DURATION ;
float timebox_scale   = 1.0f ;
float timebox_start   = 0.0f ;

#define TIMEBOX_LEFT   230.0f
#define TIMEBOX_RIGHT  790.0f
#define TIMEBOX_TOP    100.0f
#define TIMEBOX_BOTTOM  10.0f
#define TIMEBOX_WIDTH  (TIMEBOX_RIGHT-TIMEBOX_LEFT)
#define TIMEBOX_SECOND (timebox_scale * TIMEBOX_WIDTH / timebox_maxtime)

float lcursor = 0.0f ;
float rcursor = 0.0f ;
int scroll_controllers = 0 ;
int curr_button = 0 ;

#define MODE_ADD      0
#define MODE_SELECT   1

int event_mode = MODE_SELECT ;
Event *curr_event = NULL ;

unsigned int floor_texhandle  = 0 ;
float floor_z_coord  = -1 ;

puFileSelector* file_selector = 0 ;                                                 
ssgRoot        *skinScene  = NULL ;
ssgRoot        *boneScene  = NULL ;

char lastModelFilePath [ PUSTRING_MAX ] ;
char lastModelFileName [ PUSTRING_MAX ] ;

void loadCB ( puObject * ) ;

puText     *message     ;
puButton   *hideBones   ;
puButton   *hideSkin    ;
puButton   *hideGround  ;
puSlider   *scroller    ;
puSlider   *timescroller;
puGroup    *vcr         ;
puInput   *ground_speed ;
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



void setCurrentEvent ( Event *ev )
{
  curr_event = ev ;
}

float vcr_replay_speed    = 0.0f ;
float vcr_ground_speed    = 0.0f ;
float vcr_ground_position = 0.0f ;
void vcr_fastReverse ( puObject * ) { vcr_replay_speed = -3.0f ; setCurrentEvent(NULL);}
void vcr_reverse     ( puObject * ) { vcr_replay_speed = -1.0f ; setCurrentEvent(NULL);}
void vcr_stop        ( puObject * ) { vcr_replay_speed =  0.0f ; }
void vcr_play        ( puObject * ) { vcr_replay_speed =  1.0f ; setCurrentEvent(NULL);}
void vcr_fastPlay    ( puObject * ) { vcr_replay_speed =  3.0f ; setCurrentEvent(NULL);}
void vcr_groundSpeed ( puObject *me ) { vcr_ground_speed = me->getFloatValue() ; }


void reverseRegionCB (puObject *)
{
  int nevents = 0 ;
  int i ;

  /* Count the events... */

  for ( i = 0 ; i < getNumEvents() ; i++ )
  {
    Event *ev = getEvent ( i ) ;

    float t = ev -> getTime () ;

    if ( ( lcursor < rcursor && t >= lcursor && t <= rcursor ) ||
         ( rcursor < lcursor && t >= rcursor && t <= lcursor ) )
    {
      nevents++ ;
    }
  }

  if ( nevents == 0 )
    return ;

  /* Make a list of them */

  Event **elist = new Event* [ nevents ] ;

  nevents = 0 ;

  for ( i = 0 ; i < getNumEvents() ; i++ )
  {
    Event *ev = getEvent ( i ) ;

    float t = ev -> getTime () ;

    if ( ( lcursor < rcursor && t >= lcursor && t <= rcursor ) ||
         ( rcursor < lcursor && t >= rcursor && t <= lcursor ) )
    {
      elist [ nevents ] = ev ;
      nevents++ ;
    }
  }

  for ( i = 0 ; i < nevents ; i++ )
  {
    Event *ev = elist [ i ] ;

    removeEvent ( ev ) ;

    float t = ev -> getTime () ;

    if ( lcursor > rcursor )
      t = lcursor - ( t - rcursor ) ;
    else
      t = rcursor - ( t - lcursor ) ;

    ev -> setTime ( t ) ;
    addEvent ( ev ) ;
  }

  delete [] elist ;
}


void deleteAll ()
{
  while ( getNumEvents() > 0 )
  {
    Event *ev = getEvent ( 0 ) ;
    removeEvent ( ev ) ;
    delete ev ;
  }

  lcursor = rcursor = 0.0f ;
  setCurrentEvent ( NULL ) ;
}


void deleteRegionCB (puObject *)
{
  int found_one = FALSE ;

  do
  {
    found_one = FALSE ;

    for ( int i = 0 ; i < getNumEvents() && ! found_one ; i++ )
    {
      Event *ev = getEvent ( i ) ;

      float t = ev -> getTime () ;

      if ( ( lcursor < rcursor && t >= lcursor && t <= rcursor ) ||
           ( rcursor < lcursor && t >= rcursor && t <= lcursor ) )
      {
        removeEvent ( ev ) ;
        delete ev ;
        found_one = TRUE ;
      }
    }

  } while ( found_one ) ;

  setCurrentEvent ( NULL ) ;
}


void zoom_nrm_CB ( puObject * )
{
  timebox_scale  = 1.0 ;
  timescroller -> hide () ;
}

void zoom_in_CB  ( puObject * )
{
  timebox_scale *= 1.5 ;
  timescroller -> reveal () ;
}

void zoom_out_CB ( puObject * )
{
  timebox_scale /= 1.5 ;

  if ( timebox_scale <= 1.0f )
  {
    timebox_scale = 1.0f ;
    timescroller -> hide () ;
  }
  else
    timescroller -> reveal () ;
}


void add_1_CB ( puObject * ) { timebox_maxtime += 1.0f ; }
void add_2_CB ( puObject * ) { timebox_maxtime += 2.0f ; }
void add_5_CB ( puObject * ) { timebox_maxtime += 5.0f ; }


void deleteRegionAndCompressCB ( puObject *me )
{
  deleteRegionCB ( me ) ;

  for ( int i = 0 ; i < getNumEvents() ; i++ )
  {
    Event *ev = getEvent ( i ) ;

    float t = ev -> getTime () ;

    if ( t > lcursor || t > rcursor )
      ev -> setTime ( t - fabs(lcursor-rcursor) ) ;
  }

  timebox_maxtime -= fabs(lcursor-rcursor) ;

  if ( lcursor > rcursor )
    lcursor = rcursor ;
  else
    rcursor = lcursor ;

  setCurrentEvent ( NULL ) ;
}


void deleteEventCB (puObject *)
{
  if ( curr_event == NULL )
    return ;

  removeEvent ( curr_event ) ;

  Event *ev = curr_event ;
  setCurrentEvent ( NULL ) ;
  delete ev ;
}


void addNewEventCB (puObject *)
{
  if ( getNumBones() <= 0 )
    return ;

  setCurrentEvent ( NULL ) ;
  event_mode = MODE_ADD ;
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
  transformModel ( boneScene, lcursor ) ;
}


void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
  ssgSetFOV  ( 60.0f, 60.0f * (float) h / (float) w ) ;
}

 
static void updateEventQueue ( int button, int x, int y, int new_click )
{
  y = puGetWindowHeight () - y ;  

  /*
    Allow user to click a little outside the box so
    we can make it easier to get the ends exactly.
  */

  if ( x > TIMEBOX_LEFT - 20 && x < TIMEBOX_RIGHT + 20 &&
       y > TIMEBOX_BOTTOM    && y < TIMEBOX_TOP )
  {
    float c = (float)(x - TIMEBOX_LEFT) ;

    if ( c < 0.0f ) c = 0.0f ;
    if ( c > TIMEBOX_WIDTH ) c = TIMEBOX_WIDTH ;

    c /= TIMEBOX_SECOND ;
    c += timebox_start ;

    if ( button == PU_RIGHT_BUTTON )
      rcursor = c ;
    else
    if ( button == PU_LEFT_BUTTON )
    {
      rcursor = lcursor = c ;

      if ( new_click )
      {
        Event *ev = findNearestEvent ( lcursor, 10.0f / TIMEBOX_SECOND ) ;
   
        if ( event_mode == MODE_ADD && getNumBones() > 0 )
        {
          setCurrentEvent ( new Event ( getNumBones(), lcursor ) ) ;
	  addEvent ( curr_event ) ;
	  event_mode = MODE_SELECT ;
	}
	else
          setCurrentEvent ( ev ) ;
      }
      else
      if ( curr_event != NULL )
      {
	removeEvent ( curr_event ) ;
	curr_event -> setTime ( lcursor ) ;
	addEvent ( curr_event ) ;
      }
    }
  }
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


void bnsavepickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 

  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = 0 ;
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
    return ;

  FILE *fd = fopen ( file_selector->getStringValue(), "wa" ) ;

  if ( fd == NULL )
    return ;

  fprintf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
                        getNumBones(), getNumEvents(), timebox_maxtime,
                        -floor_z_coord, vcr_ground_speed ) ;

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> write ( fd ) ;

  for ( i = 0 ; i < getNumEvents () ; i++ )
    getEvent ( i ) -> write ( fd ) ;

  fclose ( fd ) ;

  puDeleteObject ( file_selector ) ;
  file_selector = 0 ;
}



void bnpickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  if ( path [ 0 ] == '\0' )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = 0 ;
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
    file_selector = 0 ;
    return ;
  }

  FILE *fd = fopen ( file_selector->getStringValue(), "ra" ) ;

  if ( fd == NULL )
  {
    puDeleteObject ( file_selector ) ;
    file_selector = 0 ;
    return ;
  }

  deleteAll () ;

  int numbones, numevents ;
  float tmp_floor_z_coord ;

  /* Don't use the floor_z_coord from the file. */

  fscanf ( fd, "NUMBONES=%d NUMEVENTS=%d MAXTIME=%f Z_OFFSET=%f SPEED=%f\n",
                &numbones, &numevents,
                &timebox_maxtime, &tmp_floor_z_coord, &vcr_ground_speed ) ;

  ground_speed -> setValue ( vcr_ground_speed ) ;

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
  file_selector = 0 ;
}



void pickfn ( puObject * )
{
  char path  [ PUSTRING_MAX ] ;
  char fname [ PUSTRING_MAX ] ;

  file_selector -> getValue ( path ) ;
 
  puDeleteObject ( file_selector ) ;
  file_selector = 0 ;

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



void timescrollerCB ( puObject *ob )
{
  timebox_start = ob -> getFloatValue () *
                     ( timebox_maxtime - TIMEBOX_WIDTH / TIMEBOX_SECOND ) ;
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
  glTexCoord2f ( -30, vcr_ground_position - 30 ) ;
  glVertex3f ( -30, -30, floor_z_coord ) ;
  glTexCoord2f (  30, vcr_ground_position - 30 ) ;
  glVertex3f (  30, -30, floor_z_coord ) ;
  glTexCoord2f (  30, vcr_ground_position + 30 ) ;
  glVertex3f (  30,  30, floor_z_coord ) ;
  glTexCoord2f ( -30, vcr_ground_position + 30 ) ;
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
  static ulClock timer ;
  int i ;

  timer.update () ;

  if ( vcr_replay_speed != 0.0f )
  {
    float delta_t = vcr_replay_speed * timer.getDeltaTime () ;

    lcursor += delta_t ;

    if ( lcursor < 0.0f )
      lcursor = timebox_maxtime  ;

    if ( lcursor > timebox_maxtime  )
      lcursor = 0.0f ;
  }

  vcr_ground_position = lcursor * vcr_ground_speed ;

  update_motion () ;

  glClear  ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  glEnable ( GL_DEPTH_TEST ) ;

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

    if ( curr_event == NULL )
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

  int w = puGetWindowWidth  () ;
  int h = puGetWindowHeight () ;  

  glPushAttrib   ( GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;
 
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;  

  glViewport     ( 0, 0, w, h ) ;  
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  gluOrtho2D     ( 0, w, 0, h ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;                                                           

  glColor4f ( 0.8f, 0.8f, 0.8f, 1.0f ) ;
  glBegin ( GL_LINE_LOOP ) ;
  glVertex2f ( TIMEBOX_LEFT , TIMEBOX_BOTTOM ) ;
  glVertex2f ( TIMEBOX_RIGHT, TIMEBOX_BOTTOM ) ;
  glVertex2f ( TIMEBOX_RIGHT, TIMEBOX_TOP    ) ;
  glVertex2f ( TIMEBOX_LEFT , TIMEBOX_TOP    ) ;
  glEnd   () ;

  int nseconds = (int)floor(TIMEBOX_WIDTH / TIMEBOX_SECOND) ;
  int ntenthseconds = (int)floor(TIMEBOX_WIDTH*10.0f / TIMEBOX_SECOND) ;
  int tenthsecondoffset = (int)( (timebox_start*10.0f -
                                  floor(timebox_start*10.0f)) 
                                     * TIMEBOX_SECOND / 10.0f ) ;
  int secondoffset = (int)( (timebox_start -
                             floor(timebox_start))
                                     * TIMEBOX_SECOND ) ;

  glBegin ( GL_LINES ) ;

  for ( i = 0 ; i < ntenthseconds ; i++ )
  {
    float x = TIMEBOX_LEFT + tenthsecondoffset +
                                 (float) i * TIMEBOX_SECOND / 10.0f ;

    glVertex2f ( x, TIMEBOX_BOTTOM ) ;
    glVertex2f ( x, TIMEBOX_TOP/3.0f ) ;
  }

  for ( i = 0 ; i < nseconds ; i++ )
  {
    float x = TIMEBOX_LEFT + secondoffset +
                                 (float) i * TIMEBOX_SECOND ;

    glVertex2f ( x, TIMEBOX_BOTTOM ) ;
    glVertex2f ( x, TIMEBOX_TOP/2.0f ) ;
  }

  glEnd () ;

  float tlcursor = ( lcursor - timebox_start ) * TIMEBOX_SECOND ;
  float trcursor = ( rcursor - timebox_start ) * TIMEBOX_SECOND ;

  tlcursor = (tlcursor <      0.0f    ) ? 0.0f :
             (tlcursor > TIMEBOX_WIDTH) ? TIMEBOX_WIDTH : tlcursor ;

  trcursor = (trcursor <      0.0f    ) ? 0.0f :
             (trcursor > TIMEBOX_WIDTH) ? TIMEBOX_WIDTH : trcursor ;

  if ( tlcursor != trcursor )
  {
    glColor4f ( 1.0f, 1.0f, 0.0f, 0.2f ) ;
    glBegin ( GL_QUADS ) ;
    glVertex2f ( TIMEBOX_LEFT + tlcursor, TIMEBOX_BOTTOM ) ;
    glVertex2f ( TIMEBOX_LEFT + tlcursor, TIMEBOX_TOP    ) ;
    glVertex2f ( TIMEBOX_LEFT + trcursor, TIMEBOX_TOP    ) ;
    glVertex2f ( TIMEBOX_LEFT + trcursor, TIMEBOX_BOTTOM ) ;
    glEnd   () ;
  }

  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;

  glBegin ( GL_LINES ) ;
    glVertex2f ( TIMEBOX_LEFT + trcursor, TIMEBOX_BOTTOM ) ;
    glVertex2f ( TIMEBOX_LEFT + trcursor, TIMEBOX_TOP    ) ;
  glEnd   () ;

  glColor4f ( 1.0f, 0.0f, 0.0f, 1.0f ) ;

  glBegin ( GL_LINES ) ;
    glVertex2f ( TIMEBOX_LEFT + tlcursor, TIMEBOX_BOTTOM ) ;
    glVertex2f ( TIMEBOX_LEFT + tlcursor, TIMEBOX_TOP    ) ;
  glEnd   () ;

  float t ;

  glColor4f ( 1.0f, 0.0f, 1.0f, 1.0f ) ;

  glBegin ( GL_LINES ) ;

  for ( i = 0 ; i < getNumEvents() ; i++ )
  {
    t = getEvent(i) -> getTime () ;

    t = ( t - timebox_start ) * TIMEBOX_SECOND ;

    if ( t >= 0.0f && t <= TIMEBOX_WIDTH )
    {
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP/3.0f ) ;
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP      ) ;
    }
  }
  glEnd () ;

  if ( curr_event != NULL )
  {
    t = curr_event -> getTime () ;

    t = ( t - timebox_start ) * TIMEBOX_SECOND ;

    if ( t >= 0.0f && t <= TIMEBOX_WIDTH )
    {
      static ulClock flasher ;
      flasher.update () ;

      if ( (int)(flasher.getAbsTime()*3.0f) & 1 )
        glColor4f ( 1.0f, 0.0f, 1.0f, 1.0f ) ;
      else
        glColor4f ( 0.0f, 1.0f, 0.0f, 1.0f ) ;
      glBegin ( GL_LINES ) ;
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP/3.0f ) ;
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP      ) ;
      glEnd () ;
    }
  }

  glMatrixMode   ( GL_PROJECTION ) ;
  glPopMatrix    () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPopMatrix    () ;
  glPopAttrib    () ;

  glutPostRedisplay () ;
  glutSwapBuffers   () ;
}


/* Menu bar entries: */
 
char      *file_submenu    [] = {  "Exit", 
                                   "------------", 
                                   "Save Bones As...",
                                   "Load Bones",
                                   "Load Model",  NULL } ;
puCallback file_submenu_cb [] = {  exitCB,
                                   NULL,
                                   bnsaveCB,
                                   bnloadCB,
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

  timescroller =  new puSlider  ( TIMEBOX_LEFT, TIMEBOX_TOP+5,
                                    570-TIMEBOX_LEFT, FALSE ) ;
  timescroller -> setCBMode     ( PUSLIDER_DELTA   ) ;
  timescroller -> setDelta      ( 0.01             ) ;
  timescroller -> setCallback   ( timescrollerCB   ) ;
  timescroller -> hide () ;

  scroller    =  new puSlider  ( 5, 70, 400, TRUE ) ;
  scroller    -> setCBMode     ( PUSLIDER_DELTA   ) ;
  scroller    -> setDelta      ( 0.01             ) ;
  scroller    -> setCallback   ( scrollerCB       ) ;

  hideBones   =  new puButton  ( 7, 0, "Bones" ) ;
  hideBones   -> setValue      ( TRUE ) ;
  hideSkin    =  new puButton  ( 46, 0, "Skin" ) ;
  hideSkin    -> setValue      ( FALSE ) ;
  hideGround  =  new puButton  ( 76, 0, "Floor" ) ;
  hideGround  -> setValue      ( FALSE ) ;

  rangeSlider =  new puSlider  ( 10, 20, 100, FALSE ) ;
  rangeSlider -> setCBMode     ( PUSLIDER_DELTA ) ;
  rangeSlider -> setDelta      ( 0.01    ) ;
  message     =  new puText    ( 10, 40 ) ;
  message     -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message     -> setLabel      ( "Zoom"  ) ;

    panSlider =  new puDial    ( 110, 0, 40 ) ;
    panSlider -> setCBMode     ( PUSLIDER_DELTA ) ;
    panSlider -> setDelta      ( 0.01f   ) ;
    panSlider -> setValue      ( 0.5f    ) ;
  message     =  new puText    ( 110, 40 ) ;
  message     -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message     -> setLabel      ( "Pan"  ) ;

   tiltSlider =  new puDial    ( 150, 0, 40 ) ;
   tiltSlider -> setCBMode     ( PUSLIDER_DELTA ) ;
   tiltSlider -> setDelta      ( 0.01f   ) ;
   tiltSlider -> setValue      ( 0.5f    ) ;
  message     =  new puText    ( 150, 40 ) ;
  message     -> setColour     ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message     -> setLabel      ( "Tilt"  ) ;

  vcr = new puGroup ( 579, TIMEBOX_TOP + 5 ) ;

  puOneShot *oneshot ;

  oneshot = new puArrowButton (  0, 0, 30, 30, PUARROW_FASTLEFT  ) ;
  oneshot -> setCallback ( vcr_fastReverse ) ;
  oneshot = new puArrowButton ( 30, 0, 60, 30, PUARROW_LEFT      ) ;
  oneshot -> setCallback ( vcr_reverse ) ;
  oneshot = new puArrowButton ( 60, 0, 90, 30, PUARROW_DOWN      ) ;
  oneshot -> setCallback ( vcr_stop ) ;
  oneshot = new puArrowButton ( 90, 0,120, 30, PUARROW_RIGHT     ) ;
  oneshot -> setCallback ( vcr_play ) ;
  oneshot = new puArrowButton (120, 0,150, 30, PUARROW_FASTRIGHT ) ;
  oneshot -> setCallback ( vcr_fastPlay ) ;

  ground_speed = new puInput ( 150, 0, 200, 20 ) ;
  ground_speed -> setCallback ( vcr_groundSpeed ) ;
  ground_speed -> setValue ( "0.0" ) ;

  message = new puText ( 150, 20 ) ;
  message -> setColour    ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message -> setLabel ( "GrndSpeed"  ) ;
  vcr -> close () ;

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




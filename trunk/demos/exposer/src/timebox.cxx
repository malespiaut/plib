#include "exposer.h"

#define MODE_ADD      0
#define MODE_SELECT   1
 
int    event_mode = MODE_SELECT ;                                               

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

puSlider *timescroller ;
puGroup  *vcr          ;
puInput  *ground_speed ;

float vcr_replay_speed    = 0.0f ;
float vcr_ground_speed    = 0.0f ;
float vcr_ground_position = 0.0f ;

float getCursor () { return lcursor ; }
float getMaxTime () { return timebox_maxtime ; }
void  setMaxTime ( float t ) { timebox_maxtime = t ; }
float getVCRGroundSpeed () { return vcr_ground_speed ; }
void  setVCRGroundSpeed ( float t ) { vcr_ground_speed = t ; ground_speed -> setValue ( vcr_ground_speed ) ; }

float getVCRGroundPosition () { return vcr_ground_position ; }
void  setVCRGroundPosition ( float t ) { vcr_ground_position = t ; }

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
  if ( getCurrentEvent() == NULL )
    return ;

  removeEvent ( getCurrentEvent() ) ;

  Event *ev = getCurrentEvent() ;
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


void timescrollerCB ( puObject *ob )
{
  timebox_start = ob -> getFloatValue () *
                     ( timebox_maxtime - TIMEBOX_WIDTH / TIMEBOX_SECOND ) ;
}


void updateVCR ()
{
  static ulClock timer ;

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
}

void drawTimeBox ()
{
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
  int i ;

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

  if ( getCurrentEvent() != NULL )
  {
    t = getCurrentEvent() -> getTime () ;

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
}



 
void updateEventQueue ( int button, int x, int y, int new_click )
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
	  addEvent ( getCurrentEvent() ) ;
	  event_mode = MODE_SELECT ;
	}
	else
          setCurrentEvent ( ev ) ;
      }
      else
      if ( getCurrentEvent() != NULL )
      {
	removeEvent ( getCurrentEvent() ) ;
	getCurrentEvent() -> setTime ( lcursor ) ;
	addEvent ( getCurrentEvent() ) ;
      }
    }
  }
}


void initTimeBox ()
{
  timescroller =  new puSlider  ( TIMEBOX_LEFT, TIMEBOX_TOP+5,
                                  570-TIMEBOX_LEFT, FALSE ) ;
  timescroller -> setCBMode     ( PUSLIDER_DELTA   ) ;
  timescroller -> setDelta      ( 0.01             ) ;
  timescroller -> setCallback   ( timescrollerCB   ) ;
  timescroller -> hide () ; 

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
 
  puText *message ;
  message = new puText ( 150, 20 ) ;
  message -> setColour ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message -> setLabel  ( "GrndSpeed"  ) ;

  vcr -> close () ;
}


#include "exposer.h"

#define MODE_ADD      0
#define MODE_SELECT   1

#define TIMEBOX_LEFT   230.0f
#define TIMEBOX_RIGHT  790.0f
#define TIMEBOX_TOP    100.0f
#define TIMEBOX_BOTTOM  10.0f
#define TIMEBOX_WIDTH  (TIMEBOX_RIGHT-TIMEBOX_LEFT)
#define TIMEBOX_INITIAL_DURATION   10.0f   /* Seconds */

/* GUI callback functions. */

static void vcr_fastReverse ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setReplaySpeed (-3.0f);
}

static void vcr_reverse ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setReplaySpeed (-1.0f);
}

static void vcr_stop ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setReplaySpeed ( 0.0f);
}

static void vcr_play ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setReplaySpeed ( 1.0f);
}

static void vcr_fastPlay ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setReplaySpeed ( 3.0f);
}

static void vcr_groundSpeed ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;
  timebox -> setGroundSpeed ( ob -> getFloatValue () ) ;
}                                                              


static void timescrollerCB ( puObject *ob )
{
  TimeBox *timebox = (TimeBox *) ob -> getUserData () ;

  timebox -> setScroll ( ob -> getFloatValue () *
                        ( timebox -> getMaxTime() -
                        TIMEBOX_WIDTH / timebox -> second () ) ) ;
}


float TimeBox::second ()
{
  return scale * TIMEBOX_WIDTH / maxtime ;
}


void TimeBox::deleteRegionAndCompress ()
{
  eventList -> compressEventsBetween ( lcursor, rcursor ) ;

  maxtime -= fabs(lcursor-rcursor) ;

  if ( lcursor > rcursor )
    lcursor = rcursor ;
  else
    rcursor = lcursor ;
}


void TimeBox::addNewEvent ()
{
  if ( getNumBones() <= 0 )
    return ;

  eventList->setCurrentEvent ( NULL ) ;
  event_mode = MODE_ADD ;
}


void TimeBox::updateVCR ()
{
  static ulClock timer ;

  timer.update () ;

  if ( replay_speed != 0.0f )
  {
    float delta_t = replay_speed * timer.getDeltaTime () ;

    lcursor += delta_t ;

    if ( lcursor < 0.0f )
      lcursor = maxtime  ;

    if ( lcursor > maxtime  )
      lcursor = 0.0f ;
  }

  ground_position = lcursor * ground_speed ;
}


void TimeBox::draw ()
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

  int nseconds = (int)floor(TIMEBOX_WIDTH / second () ) ;
  int ntenthseconds = (int)floor(TIMEBOX_WIDTH*10.0f / second () ) ;
  int tenthsecondoffset = (int)( (start*10.0f -
                                  floor(start*10.0f)) 
                                     * second ()  / 10.0f ) ;
  int secondoffset = (int)( (start -
                             floor(start))
                                     * second ()  ) ;
  int i ;

  glBegin ( GL_LINES ) ;

  for ( i = 0 ; i < ntenthseconds ; i++ )
  {
    float x = TIMEBOX_LEFT + tenthsecondoffset +
                                 (float) i * second ()  / 10.0f ;

    glVertex2f ( x, TIMEBOX_BOTTOM ) ;
    glVertex2f ( x, TIMEBOX_TOP/3.0f ) ;
  }

  for ( i = 0 ; i < nseconds ; i++ )
  {
    float x = TIMEBOX_LEFT + secondoffset +
                                 (float) i * second ()  ;

    glVertex2f ( x, TIMEBOX_BOTTOM ) ;
    glVertex2f ( x, TIMEBOX_TOP/2.0f ) ;
  }

  glEnd () ;

  float tlcursor = ( lcursor - start ) * second ()  ;
  float trcursor = ( rcursor - start ) * second ()  ;

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

  for ( i = 0 ; i < eventList->getNumEvents() ; i++ )
  {
    t = eventList->getEvent(i) -> getTime () ;

    t = ( t - start ) * second ()  ;

    if ( t >= 0.0f && t <= TIMEBOX_WIDTH )
    {
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP/3.0f ) ;
      glVertex2f ( TIMEBOX_LEFT + t, TIMEBOX_TOP      ) ;
    }
  }
  glEnd () ;

  if ( eventList->getCurrentEvent() != NULL )
  {
    t = eventList->getCurrentEvent() -> getTime () ;

    t = ( t - start ) * second ()  ;

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



 
void TimeBox::updateEventQueue ( int button, int x, int y, int new_click )
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

    c /= second ()  ;
    c += start ;

    if ( button == PU_RIGHT_BUTTON )
      rcursor = c ;
    else
    if ( button == PU_LEFT_BUTTON )
    {
      rcursor = lcursor = c ;

      if ( new_click )
      {
        if ( event_mode == MODE_ADD )
          eventList -> newEvent ( lcursor ) ;
	else
          eventList -> setCurrentEvent (
               eventList -> findNearestEvent ( lcursor, 10.0f/second () ) ) ;

        event_mode = MODE_SELECT ;
      }
      else
      if ( eventList->getCurrentEvent() != NULL )
	eventList->moveEvent ( eventList->getCurrentEvent(), lcursor ) ;
    }
  }
}


void TimeBox::init ()
{
  event_mode = MODE_SELECT ;                                               

  maxtime = TIMEBOX_INITIAL_DURATION ;
  scale   = 1.0f ;
  start   = 0.0f ;

  lcursor = 0.0f ;
  rcursor = 0.0f ;

  replay_speed    = 0.0f ;
  ground_speed    = 0.0f ;
  ground_position = 0.0f ;

  timescroller =  new puSlider  ( TIMEBOX_LEFT, TIMEBOX_TOP+5,
                                  570-TIMEBOX_LEFT, FALSE ) ;
  timescroller -> setCBMode     ( PUSLIDER_DELTA   ) ;
  timescroller -> setDelta      ( 0.01             ) ;
  timescroller -> setCallback   ( timescrollerCB   ) ;
  timescroller -> setUserData   ( this             ) ;
  timescroller -> hide () ; 

  puText    *message ;
  puOneShot *oneshot ;
  puGroup   *vcr = new puGroup ( 579, TIMEBOX_TOP + 5 ) ;
 
  oneshot = new puArrowButton (  0, 0, 30, 30, PUARROW_FASTLEFT  ) ;
  oneshot -> setCallback ( vcr_fastReverse ) ;
  oneshot -> setUserData ( this ) ;
  oneshot = new puArrowButton ( 30, 0, 60, 30, PUARROW_LEFT      ) ;
  oneshot -> setCallback ( vcr_reverse ) ;
  oneshot -> setUserData ( this ) ;
  oneshot = new puArrowButton ( 60, 0, 90, 30, PUARROW_DOWN      ) ;
  oneshot -> setCallback ( vcr_stop ) ;
  oneshot -> setUserData ( this ) ;
  oneshot = new puArrowButton ( 90, 0,120, 30, PUARROW_RIGHT     ) ;
  oneshot -> setCallback ( vcr_play ) ;
  oneshot -> setUserData ( this ) ;
  oneshot = new puArrowButton (120, 0,150, 30, PUARROW_FASTRIGHT ) ;
  oneshot -> setCallback ( vcr_fastPlay ) ;
  oneshot -> setUserData ( this ) ;
 
  ground_speed_input = new puInput  ( 150, 0, 200, 20 ) ;
  ground_speed_input -> setCallback ( vcr_groundSpeed ) ;
  ground_speed_input -> setValue    ( "0.0" ) ;
  ground_speed_input -> setUserData ( this ) ;
 
  message = new puText ( 150, 20 ) ;
  message -> setColour ( PUCOL_LABEL, 0.7f,0.65f,0.26f,1 ) ;
  message -> setLabel  ( "GrndSpeed"  ) ;

  vcr -> close () ;
}



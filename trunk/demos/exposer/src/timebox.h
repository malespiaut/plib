

class TimeBox
{
  int   event_mode ;

  float maxtime ;
  float scale   ;
  float start   ;
  float lcursor ;
  float rcursor ;
 
  EventList *eventList ;
  puSlider  *timescroller ;
  puInput   *ground_speed_input ;
 
  float replay_speed    ;
  float ground_speed    ;
  float ground_position ;

  void init () ;
  
public:

  void setReplaySpeed ( float spd )
  {
    replay_speed = spd ;
    eventList->setCurrentEvent ( NULL ) ;
  }

  void  draw () ;

  void  updateEventQueue ( int button, int x, int y, int new_click ) ;
  void  updateVCR () ;

  TimeBox ( EventList *el ) { eventList = el ; init () ; }

  float second () ;
 
  float getCursorTime () { return lcursor ; }

  void  setMaxTime ( float t ) { maxtime = t ; }
  float getMaxTime () { return maxtime ; }


  void  setGroundSpeed ( float t )
  {
    ground_speed = t ;
    ground_speed_input -> setValue ( ground_speed ) ;
  }
  float getGroundSpeed () { return ground_speed ; }
   
  void  setGroundPosition ( float t ) { ground_position = t ; }
  float getGroundPosition () { return ground_position ; }

  void  setScroll ( float z ) { start = z ; }
  float getScroll () { return start ; }

  void setZoom ( float z )
  {
    scale = z ;

    if ( scale == 1.0f )
      timescroller -> hide () ;
    else
      timescroller -> reveal () ;
  }
  float getZoom () { return scale ; }

  void reverseRegion ()
  {
    eventList -> reverseEventsBetween ( lcursor, rcursor ) ;
  }

  void deleteAll ()
  {
    eventList -> deleteAll () ;
    lcursor = rcursor = 0.0f ;
  }

  void deleteRegion ()
  {
    eventList -> deleteEventsBetween ( lcursor, rcursor ) ;
  }

  void deleteEvent ()
  {
    eventList -> deleteCurrentEvent () ;
  }

  void deleteRegionAndCompress () ;
  void addNewEvent () ;
} ;



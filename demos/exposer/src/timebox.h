

class TimeBox
{
  int   event_mode ;
  float timebox_maxtime ;
  float timebox_scale   ;
  float timebox_start   ;
  float lcursor ;
  float rcursor ;
 
  puSlider *timescroller ;
  puInput  *ground_speed ;
 
  float vcr_replay_speed    ;
  float vcr_ground_speed    ;
  float vcr_ground_position ;

  void init () ;
  
public:

  void setVCRReplaySpeed ( float spd )
  {
    vcr_replay_speed = spd ;
    setCurrentEvent ( NULL ) ;
  }

  void  draw () ;

  void  updateEventQueue ( int button, int x, int y, int new_click ) ;
  void  updateVCR () ;

  TimeBox () { init () ; }

  float second () ;
 
  float getCursorTime () { return lcursor ; }

  void  setMaxTime ( float t ) { timebox_maxtime = t ; }
  float getMaxTime () { return timebox_maxtime ; }


  void  setVCRGroundSpeed ( float t )
  {
    vcr_ground_speed = t ;
    ground_speed -> setValue ( vcr_ground_speed ) ;
  }
  float getVCRGroundSpeed () { return vcr_ground_speed ; }
   
  void  setVCRGroundPosition ( float t ) { vcr_ground_position = t ; }
  float getVCRGroundPosition () { return vcr_ground_position ; }

  void  setScroll ( float z ) { timebox_start = z ; }
  float getScroll () { return timebox_start ; }

  void setZoom ( float z )
  {
    timebox_scale = z ;

    if ( timebox_scale == 1.0f )
      timescroller -> hide () ;
    else
      timescroller -> reveal () ;
  }
  float getZoom () { return timebox_scale ; }

  void reverseRegion () ;
  void deleteAll     () ;
  void deleteRegion  () ;
  void deleteRegionAndCompress () ;
  void deleteEvent   () ;
  void addNewEvent   () ;

} ;




class Event
{
  float    time ;
  int      nbones ;
  sgCoord *bone_angles ;
  sgVec3   translate ;

public:

   Event ( int n, float t ) ;
  ~Event () { delete [] bone_angles ; }

  void read  ( FILE *fd ) ;
  void write ( FILE *fd ) ;

  void setTranslate ( sgVec3 tra ) { sgCopyVec3 ( translate, tra ) ; } 
  void getTranslate ( sgVec3 tra ) { sgCopyVec3 ( tra, translate ) ; }

  void setBoneAngles ( int n, sgVec3 angles )
  {
    if ( n < nbones )
      sgCopyVec3 ( bone_angles[n].hpr, angles ) ;
  }

  void getBoneAngles ( int n, sgVec3 angles )
  {
    if ( n < nbones )
      sgCopyVec3 ( angles, bone_angles[n].hpr ) ;
    else
      sgZeroVec3 ( angles ) ;
  }

  sgCoord *getBoneCoord ( int n ) { return & ( bone_angles [ n ] ) ; }

  void  setTime ( float t ) { time = t ; }
  float getTime () { return time ; }
} ;





class EventList : public ulList
{
  Event *curr_event ;

public:

  EventList () ;

  int    getNumEvents     () { return getNumEntities () ; }

  void   addEvent         ( Event *e ) ;
  void   newEvent         ( float t ) ;
  Event *findNearestEvent ( float t, float tolerance ) ;

  void   moveEvent        ( Event *e, float new_time ) ;
  void   removeEvent      ( Event *e  ) { removeEntity ( e ) ; }
  void   setCurrentEvent  ( Event *ev ) { curr_event = ev ; }
  Event *getEvent         ( int    i  ) { return (Event *) getEntity ( i ) ; }
  void   deleteEvent      ( Event *e  ) ;
  void   deleteAll        () ;

  Event *getCurrentEvent   () { return curr_event ; }
  void   deleteCurrentEvent() { deleteEvent ( curr_event ) ; }

  int    getNumEventsBetween  ( float t1, float t2 ) ;
  int    getEventsBetween     ( float t1, float t2, Event ***elist ) ;
  void   reverseEventsBetween ( float t1, float t2 ) ;
  void   deleteEventsBetween  ( float t1, float t2 ) ;
  void   compressEventsBetween( float t1, float t2 ) ;

  void   write ( FILE *fd ) ;
  void   read  ( int nevents, FILE *fd ) ;
} ;




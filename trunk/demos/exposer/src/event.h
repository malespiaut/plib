
class Event
{
  float time ;

  int nbones ;
  sgCoord *bone_angles ;

  sgVec3 translate ;

public:

  void read  ( FILE *fd ) ;
  void write ( FILE *fd ) ;

  void setTranslate ( sgVec3 tra )
  {
    sgCopyVec3 ( translate, tra ) ;
  }

  void getTranslate ( sgVec3 tra )
  {
    sgCopyVec3 ( tra, translate ) ;
  }

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

  sgCoord *getBoneCoord ( int n )
  {
    return & ( bone_angles [ n ] ) ;
  }

  void  setTime ( float t ) { time = t ; }
  float getTime () { return time ; }

  Event ( int n, float t ) ;
  ~Event () { delete [] bone_angles ; }
} ;


extern ulList *eventList ;
extern Event  *curr_event ;

void   addEvent         ( Event *e ) ;
void   removeEvent      ( Event *e ) ;
Event *findNearestEvent ( float t, float tolerance ) ;
void   init_events      () ;

int getNumEvents () ;
Event *getEvent ( int i ) ;


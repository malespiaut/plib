
#include "exposer.h"

ulList *eventList ;


int    getNumEvents () { return eventList->getNumEntities () ; }
Event *getEvent ( int i ) { return (Event*)(eventList->getEntity(i)) ; }


void addEvent ( Event *e )
{
  for ( int i = 0 ; i < getNumEvents() ; i++ )
    if ( e -> getTime() < getEvent(i)->getTime() )
    {
      eventList->addEntityBefore ( i, e ) ;
      return ;
    }

  eventList -> addEntity ( e ) ;
}




void removeEvent ( Event *e )
{
  eventList -> removeEntity ( e ) ;
}




Event *findNearestEvent ( float t, float tolerance )
{
  int nearest = -1 ;
  float min = FLT_MAX ;

  for ( int i = 0 ; i < getNumEvents() ; i++ )
  {
    float t2 = getEvent(i)->getTime() ;
    float diff = (float) fabs ( t - t2 ) ;

    if ( diff < min )
    {
      min = diff ;
      nearest = i ;
    }
  }

  return ( nearest >= 0 && min < tolerance ) ?  getEvent ( nearest ) : NULL ;
}



void init_events ()
{
  eventList = new ulList () ;
}



Event::Event ( int n, float t )
{
  time   = t ;
  nbones = n ;
  bone_angles = new sgCoord [ n ] ;

  for ( int i = 0 ; i < n ; i++ )
  {
    sgCopyVec3 ( bone_angles [ i ] . hpr, getBone(i)->getDialAngles() ) ;
    sgCopyVec3 ( bone_angles [ i ] . xyz, getBone(i)->getXYZ() ) ;
  }
}


void Event::read ( FILE *fd )
{
  int ssh, ssp, ssr ;
 
  fscanf ( fd, "EVENT %f %d\n", &time, &nbones ) ;

  for ( int i = 0 ; i < nbones ; i++ )
  {
    fscanf ( fd, "  (%d,%d,%d)\n", & ssh, & ssp, & ssr ) ;
    sgSetVec3 ( bone_angles [ i ] . hpr, ssh, ssp, ssr ) ;
    sgCopyVec3 ( bone_angles [ i ] . xyz, getBone(i)->getXYZ() ) ;
  }
}
 
 
void Event::write ( FILE *fd )
{
  fprintf ( fd, "EVENT %f %d\n", time, nbones ) ;

  for ( int i = 0 ; i < nbones ; i++ )
  {
    int ssh = (int) bone_angles [ i ] . hpr[0] ;
    int ssp = (int) bone_angles [ i ] . hpr[1] ;
    int ssr = (int) bone_angles [ i ] . hpr[2] ;
    fprintf ( fd, "  (%d,%d,%d)\n", ssh, ssp, ssr ) ;
  }
}



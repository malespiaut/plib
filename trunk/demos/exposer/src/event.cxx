#include "exposer.h"


void EventList::read ( int nevents, FILE *fd )
{
  for ( int i = 0 ; i < nevents ; i++ )
  {
    Event *e = new Event ( getNumBones(), (float) i ) ;
    e -> read ( fd ) ;
    addEvent ( e ) ;
  }
}


void EventList::write ( FILE *fd )
{
  for ( int i = 0 ; i < getNumEvents () ; i++ )
    getEvent ( i ) -> write ( fd ) ;
}


void EventList::moveEvent ( Event *e, float new_time )
{
  removeEvent ( e ) ;
  e -> setTime ( new_time ) ;
  addEvent ( e ) ;
}


void EventList::newEvent ( float t )
{
  if ( getNumBones () <= 0 )
    return ;

  setCurrentEvent ( new Event ( getNumBones (), t ) ) ;
  addEvent ( getCurrentEvent () ) ;
}


void EventList::deleteEvent ( Event *e )
{
  if ( e == NULL )
    return ;

  if ( e == curr_event )
    curr_event = NULL ;

  removeEvent ( e ) ;
  delete e ;
}


void EventList::compressEventsBetween ( float t1, float t2 )
{
  deleteEventsBetween ( t1, t2 ) ;
 
  for ( int i = 0 ; i < getNumEvents() ; i++ )
  {
    Event *ev = getEvent ( i ) ;
 
    float t = ev -> getTime () ;
 
    if ( t > t1 || t > t2 )
      ev -> setTime ( t - fabs(t1-t2) ) ;
  }
}


int EventList::getNumEventsBetween ( float t1, float t2 )
{
  int nevents = 0 ;

  for ( int i = 0 ; i < getNumEvents() ; i++ )
  {
    float t = getEvent ( i ) -> getTime () ;
 
    if ( ( t1 < t2 && t >= t1 && t <= t2 ) ||
         ( t2 < t1 && t >= t2 && t <= t1 ) )
      nevents++ ;
  }

  return nevents ;
}


int EventList::getEventsBetween ( float t1, float t2, Event ***elist )
{
  int nevents = getNumEventsBetween ( t1, t2 ) ;
 
  if ( nevents == 0 )
  {
    elist = NULL ;
    return 0 ;
  }
 
  /* Make a list of the events */
 
  *elist = new Event* [ nevents ] ;
 
  nevents = 0 ;
 
  for ( int i = 0 ; i < getNumEvents() ; i++ )
  {
    Event *ev = getEvent ( i ) ;
    float t = ev -> getTime () ;
 
    if ( ( t1 < t2 && t >= t1 && t <= t2 ) ||
         ( t2 < t1 && t >= t2 && t <= t1 ) )
      (*elist) [ nevents++ ] = ev ;
  }

  return nevents ;
}


void EventList::deleteEventsBetween ( float t1, float t2 )
{
  int found_one = FALSE ;
 
  do
  {
    found_one = FALSE ;
 
    for ( int i = 0 ; i < getNumEvents() && ! found_one ; i++ )
    {
      Event *ev = getEvent ( i ) ;
 
      float t = ev -> getTime () ;
 
      if ( ( t1 < t2 && t >= t1 && t <= t2 ) ||
           ( t2 < t1 && t >= t2 && t <= t1 ) )
      {
        deleteEvent ( ev ) ;
        found_one = TRUE ;
      }
    }
 
  } while ( found_one ) ;
}


void EventList::reverseEventsBetween ( float t1, float t2 )
{
  /* Make a list of the events */
 
  Event **elist ;
  int nevents = getEventsBetween ( t1, t2, &elist ) ;
 
  if ( nevents == 0 )
    return ;
 
  /* Reverse their order */

  for ( int i = 0 ; i < nevents ; i++ )
  {
    Event *ev = elist [ i ] ;
 
    removeEvent ( ev ) ;
 
    float t = ev -> getTime () ;
 
    if ( t1 > t2 )
      t = t1 - ( t - t2 ) ;
    else
      t = t2 - ( t - t1 ) ;
 
    ev -> setTime ( t ) ;

    addEvent ( ev ) ;
  }
 
  delete [] elist ;
}


void EventList::addEvent ( Event *e )
{
  /*
    Keep list in time-increasing order.
  */

  for ( int i = 0 ; i < getNumEvents() ; i++ )
    if ( e -> getTime() < getEvent(i)->getTime() )
    {
      addEntityBefore ( i, e ) ;
      return ;
    }

  addEntity ( e ) ;
}



Event *EventList::findNearestEvent ( float t, float tolerance )
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


void EventList::deleteAll ()
{
  while ( getNumEvents() > 0 )
    deleteEvent ( getEvent ( 0 ) ) ;
}


EventList::EventList ()
{
  curr_event = NULL ;
}



Event::Event ( int n, float t )
{
  time   = t ;
  nbones = n ;
  bone_angles = new sgCoord [ n ] ;

  sgCopyVec3 ( translate, getCurrTranslate() ) ;

  for ( int i = 0 ; i < n ; i++ )
  {
    sgCopyVec3 ( bone_angles [ i ] . hpr, getBone(i)->getDialAngles() ) ;
    sgCopyVec3 ( bone_angles [ i ] . xyz, getBone(i)->getXYZ() ) ;
  }
}


void Event::read ( FILE *fd )
{
  fscanf ( fd, "EVENT %f %d (%f,%f,%f)\n", &time, &nbones,
                 &translate[0], &translate[1], &translate[2] ) ;

  for ( int i = 0 ; i < nbones ; i++ )
  {
    int ssh, ssp, ssr ;
 
    fscanf ( fd, "  (%d,%d,%d)\n", & ssh, & ssp, & ssr ) ;

    sgSetVec3  ( bone_angles [ i ] . hpr, ssh, ssp, ssr ) ;
    sgCopyVec3 ( bone_angles [ i ] . xyz, getBone(i)->getXYZ() ) ;
  }
}
 
 
void Event::write ( FILE *fd )
{
  fprintf ( fd, "EVENT %f %d (%f,%f,%f)\n", time, nbones,
                 translate[0], translate[1], translate[2] ) ;

  for ( int i = 0 ; i < nbones ; i++ )
  {
    fprintf ( fd, "  (%d,%d,%d)\n",
               (int) bone_angles [ i ] . hpr[0],
               (int) bone_angles [ i ] . hpr[1],
               (int) bone_angles [ i ] . hpr[2] ) ;
  }
}



/*
     This file is part of ExPoser - A Tool for Animating PLIB Critters.
     Copyright (C) 2001  Steve Baker

     ExPoser is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     ExPoser is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with ExPoser; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


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




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



/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include <plib/sl.h>
#include <plib/sm.h>
#include <math.h>

/*
  Construct a sound scheduler and a mixer.
*/

slScheduler sched ( 8000 ) ;
smMixer mixer ( "/dev/mixer" ) ;

#ifndef M_PI
#define EX_PI 3.1415926535
#else
#define EX_PI M_PI
#endif

int main ( int argc, char **argv )
{
  mixer . setMasterVolume ( 100 ) ;
  sched . setSafetyMargin ( 0.128f ) ;

  /* Set up the samples and a loop */

  slSample *s1 = new slSample ( argv[1], & sched ) ;

  s1 -> adjustVolume ( 10.0f  ) ;

  sched . loopSample ( s1 ) ;

  int tim = 0 ;  /* My periodic event timer. */

  while ( SL_TRUE )
  {
    tim++ ;  /* Time passes */

#ifdef WIN32
    Sleep ( 1000 / 30 ) ;      /* 30Hz */
#elif defined(sgi)
    sginap( 3 );               /* ARG */
#else
    usleep ( 1000000 / 30 ) ;  /* 30Hz */
#endif

    /*
      This would normally be called just before the graphics buffer swap
      - but it could be anywhere where it's guaranteed to get called
      fairly often.
    */

    sched . update () ;
  }

  return 0 ;
}



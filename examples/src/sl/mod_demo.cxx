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


slScheduler sched ( 44100 ) ;

int main ( int argc, char **argv )
{
  sched . setSafetyMargin ( 0.5 ) ;

  if ( argc == 2 )
    sched . loopMusic ( argv[1] ) ;
  else
    sched . loopMusic ( "tuxr.mod" ) ;
  

  while ( SL_TRUE )
  {
    /*
      For the sake of realism, I'll delay for 1/30th second to
      simulate a graphics update process.
    */

#ifdef WIN32
    Sleep ( 1000 / 30 ) ;      /* 30Hz */
#elif defined(sgi)
    sginap( 3 );               /* ARG */
#else
    usleep ( 1000000 / 30 ) ;  /* 30Hz */
#endif

    sched . update () ;
  }

  return 0 ;
}


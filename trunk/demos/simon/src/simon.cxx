/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2002  Steve Baker

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


#include <simon.h>
#include <plib/psl.h>

pslValue my_siJoystickUD ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickUD () ) ;
  return ret ;
}


pslValue my_siJoystickLR ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickLR () ) ;
  return ret ;
}


pslValue my_siJoystickA ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickA () ) ;
  return ret ;
}


pslValue my_siJoystickB ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickB () ) ;
  return ret ;
}


pslValue my_siJoystickC ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickC () ) ;
  return ret ;
}


pslValue my_siJoystickD ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickD () ) ;
  return ret ;
}


pslValue my_siJoystickL ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickL () ) ;
  return ret ;
}


pslValue my_siJoystickR ( int, pslValue *, pslProgram * )
{
  pslValue ret ;
  ret . set ( siJoystickR () ) ;
  return ret ;
}


pslValue my_siLoad ( int argc, pslValue *argv, pslProgram *p )
{
  if ( argc != 1 || argv[0].getType() != PSL_STRING )
    fprintf ( stderr, "siLoad: ERROR - Parameter must be a single string!\n" ) ;

  pslValue ret ;
  ret . set ( siLoad ( argv[0].getString () ) ) ;
  return ret ;
}



pslValue my_fabs ( int argc, pslValue *argv, pslProgram *p )
{
  pslValue ret ;
  ret . set ( (float) fabs ( argv[0].getFloat () ) ) ;
  return ret ;
}


pslValue my_siPosition ( int argc, pslValue *argv, pslProgram *p )
{
  if ( argc != 7 )
    fprintf ( stderr, "siPosition: ERROR - Expected seven parameters?!?\n" ) ;

  siPosition ( argv[0].getInt (),
               argv[1].getFloat (),
               argv[2].getFloat (),
               argv[3].getFloat (),
               argv[4].getFloat (),
               argv[5].getFloat (),
               argv[6].getFloat () ) ;

  pslValue ret ;
  return ret ;
}




pslExtension extensions [] =
{
  { "siLoad"      , 1, my_siLoad       },
  { "siPosition"  , 7, my_siPosition   },
  { "siJoystickUD", 0, my_siJoystickUD },
  { "siJoystickLR", 0, my_siJoystickLR },
  { "siJoystickA" , 0, my_siJoystickA  },
  { "siJoystickB" , 0, my_siJoystickB  },
  { "siJoystickC" , 0, my_siJoystickC  },
  { "siJoystickD" , 0, my_siJoystickD  },
  { "siJoystickL" , 0, my_siJoystickL  },
  { "siJoystickR" , 0, my_siJoystickR  },
  { "fabs"        , 1, my_fabs         },
  { NULL, 0, NULL }
} ;


pslProgram *prog = NULL ;
ulClock ck ;

int main ( int argc, char **argv )
{
  if ( argc != 2 )
  {
    fprintf ( stderr, "simon: Usage -\n\n" ) ;
    fprintf ( stderr, "        simon filename\n\n" ) ;
    fprintf ( stderr, "Where: 'filename' is a PSL sourcefile\n\n" ) ;
    exit ( 1 ) ;
  }

  pslInit () ;

  /* Create program and compile it. */

  prog = new pslProgram ( extensions, argv[1] ) ;

  ck.setMaxDelta ( 100000.0 ) ;
  ck.update () ;

  prog -> compile ( argv[1] ) ;

  ck.update () ;
  fprintf(stderr, "Compile Time: %fs.\n", ck.getDeltaTime () ) ;

  siRun () ;
  return 0 ;
}


void siUpdate ()
{
  static int nframes = 0 ;
  static double total_time = 0.0 ;

  pslResult res ;

  ck.update () ;
  do
  {
    res = prog -> step () ;

  } while ( res == PSL_PROGRAM_CONTINUE ) ;

  if ( res == PSL_PROGRAM_END )
    exit ( 0 ) ;

  ck.update () ;
  total_time += ck.getDeltaTime () ;
  nframes++ ;

  if ( nframes == 100 )
  {
    fprintf ( stderr, "Avg PSL Interp time = %fs.\n", total_time / nframes ) ;
    nframes = 0 ;
    total_time = 0.0 ;
  }
}



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


#include <plib/js.h>

int main ( int, char ** )
{
  jsJoystick *js[2] ;
  float      *ax[2] ;

  jsInit () ;

  js[0] = new jsJoystick ( 0 ) ;
  js[1] = new jsJoystick ( 1 ) ;

  printf ( "Joystick test program.\n" ) ;
  printf ( "~~~~~~~~~~~~~~~~~~~~~~\n" ) ;

  if ( js[0]->notWorking () )
    printf ( "Joystick 0 not detected\n" ) ;
  else
    printf ( "Joystick 0 is \"%s\"\n", js[0]->getName() ) ;

  if ( js[1]->notWorking () )
    printf ( "Joystick 1 not detected\n" ) ;
  else
    printf ( "Joystick 1 is \"%s\"\n", js[1]->getName() ) ;

  if ( js[0]->notWorking () && js[1]->notWorking () ) exit ( 1 ) ;

  ax[0] = new float [ js[0]->getNumAxes () ] ;
  ax[1] = new float [ js[1]->getNumAxes () ] ;

  int i, j ;

  for ( i = 0 ; i < 2 ; i++ )
    printf ( "+---------------JS.%d-----------------", i ) ;

  printf ( "+\n" ) ;

  for ( i = 0 ; i < 2 ; i++ )
  {
    if ( js[i]->notWorking () )
      printf ( "|       ~~~ Not Detected ~~~         " ) ;
    else
    {
      printf ( "| Btns " ) ;

      for ( j = 0 ; j < js[i]->getNumAxes () ; j++ )
        printf ( "Ax:%d ", j ) ;

      for ( ; j < 6 ; j++ )
        printf ( "     " ) ;
    }
  }

  printf ( "|\n" ) ;

  for ( i = 0 ; i < 2 ; i++ )
    printf ( "+------------------------------------" ) ;

  printf ( "+\n" ) ;

  while (1)
  {
    for ( i = 0 ; i < 2 ; i++ )
    {
      if ( js[i]->notWorking () )
        printf ( "|  .   .   .   .   .   .   .   .   . " ) ;
      else
      {
        int b ;

        js[i]->read ( &b, ax[i] ) ;

        printf ( "| %04x ", b ) ;

	for ( j = 0 ; j < js[i]->getNumAxes () ; j++ )
	  printf ( "%+.1f ", ax[i][j] ) ;

	for ( ; j < 6 ; j++ )
	  printf ( "  .  " ) ;
      }
    }

    printf ( "|\r" ) ;
    fflush ( stdout ) ;

    /* give other processes a chance */

#ifdef WIN32
    Sleep ( 1 ) ;
#elif defined(sgi)
    sginap ( 1 ) ;
#else
    usleep ( 1000 ) ;
#endif
  }

  return 0 ;
}



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


#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <plib/psl.h>


PSL_Variable hello ( int argc, PSL_Variable *argv, PSL_Program *p )
{
  printf ( "I am %s.\n", (char *)( p->getUserData ()) ) ;

  PSL_Variable ret ;
  ret.f = 0.0f ;

  return ret ;
}


PSL_Extension extensions [] =
{
  { "hello", 0, hello },
  { NULL, 0, NULL }
} ;


int main ()
{
  /* Create program 1 and compile it. */

  PSL_Program *prog_1 = new PSL_Program ( extensions ) ;

  prog_1 -> parse ( "data/test.psl" ) ;
  prog_1 -> dump () ;

  /* Clone program 2 from program 1 */

  PSL_Program *prog_2 = new PSL_Program ( prog_1 ) ;

  /* Make them unique by assigning user data to them */

  prog_1 -> setUserData ( (void *) "Program 1" ) ;
  prog_2 -> setUserData ( (void *) "Program 2" ) ;

  /* Run both programs together until one of them ends */

  while ( 1 )
  {
    if ( prog_1 -> step () == PSL_PROGRAM_END ) break ;
    if ( prog_2 -> step () == PSL_PROGRAM_END ) break ;
  }

  exit ( 0 ) ;
}



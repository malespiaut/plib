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

pslValue my_printf ( int argc, pslValue *argv, pslProgram *p )
{
  if ( argv[0].getType() != PSL_STRING )
    printf ( "printf: ERROR - First param must be a string!\n" ) ;
  else
  {
    printf ( "%s", argv[0].getString() ) ;

    for ( int i = 1 ; i < argc ; i++ )
    {
      switch ( argv[i].getType () )
      { 
        case PSL_INT    : printf ( "%d ", argv[i].getInt    () ) ; break ;
        case PSL_FLOAT  : printf ( "%f ", argv[i].getFloat  () ) ; break ;
        case PSL_STRING : printf ( "%s ", argv[i].getString () ) ; break ;
        case PSL_VOID   : printf ( "(void) " ) ; break ;
      }
    }
  }

  pslValue ret ;
  return ret ;
}


pslValue identify ( int argc, pslValue *argv, pslProgram *p )
{
  printf ( "I am %s.\n", (char *)( p->getUserData ()) ) ;

  pslValue ret ;
  return ret ;
}


pslExtension extensions [] =
{
  { "identify",  0, identify },
  { "printf", -1, my_printf },
  { NULL, 0, NULL }
} ;


int main ()
{
  pslInit () ;

  /* Create program 1 and compile it. */

  pslProgram *prog_1 = new pslProgram ( extensions, "code1" ) ;

  ulClock ck ;
  ck.setMaxDelta ( 100000.0 ) ;
  ck.update () ;

  prog_1 -> compile ( "data/test.psl" ) ;

  ck.update () ;
  fprintf(stderr, "%fs compiletime elapsed\n", ck.getDeltaTime () ) ;

  prog_1 -> dump () ;

  /* Clone program 2 from program 1 */

  pslProgram *prog_2 = new pslProgram ( prog_1, "code2" ) ;

  /* Make program 3 from inline strings. */

  pslProgram *prog_3 = new pslProgram ( extensions, "inline" ) ;

  prog_3 -> compile ( "int main () { printf ( \"Hello World.\\n\" ) ; }\n",
                      "HelloProgram" ) ;

  /* Make them unique by assigning user data to them */

  prog_1 -> setUserData ( (void *) "Program 1" ) ;
  prog_2 -> setUserData ( (void *) "Program 2" ) ;
  prog_3 -> setUserData ( (void *) "Program 3" ) ;

  ck.update () ;

  while ( 1 )
  {
    if ( prog_1 -> step () == PSL_PROGRAM_END &&
         prog_2 -> step () == PSL_PROGRAM_END &&
         prog_3 -> step () == PSL_PROGRAM_END )
      break ;
  }
  ck.update () ;
  fprintf(stderr, "%fs runtime elapsed\n", ck.getDeltaTime () ) ;

  exit ( 0 ) ;
}



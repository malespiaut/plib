
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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



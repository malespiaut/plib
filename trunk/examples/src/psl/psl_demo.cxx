
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <plib/psl.h>

/* EMPTY FOR NOW */

float hello ( int argc, float *argv )
{
  printf ( "Hello World\n" ) ;
  return 0.0f ;
}

PSL_Extension extensions [] =
{
  { "hello", 0, hello },
  { NULL, 0, NULL }
} ;


int main ()
{
  PSL_Program *prog = new PSL_Program ( extensions ) ;

  prog -> parse ( "data/test.psl" ) ;

  prog -> dump () ;

  while ( prog -> step () != PSL_PROGRAM_END )
    /* Do nothing */ ;

  exit ( 0 ) ;
}



#include <simon.h>
#include <plib/psl.h>

pslValue my_siLoad ( int argc, pslValue *argv, pslProgram *p )
{
  if ( argc != 1 || argv[0].getType() != PSL_STRING )
    fprintf ( stderr, "siLoad: ERROR - Parameter must be a single string!\n" ) ;

  pslValue ret ;
  ret . set ( siLoad ( argv[0].getString () ) ) ;
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
  { "siLoad"    ,  1, my_siLoad     },
  { "siPosition",  7, my_siPosition },
  { NULL, 0, NULL }
} ;


pslProgram *prog = NULL ;

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

  ulClock ck ;
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
  pslResult res ;

  do
  {
    res = prog -> step () ;

  } while ( res == PSL_PROGRAM_CONTINUE ) ;

  if ( res == PSL_PROGRAM_END )
    exit ( 0 ) ;
}



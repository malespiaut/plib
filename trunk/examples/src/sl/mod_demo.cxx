

#include <plib/sl.h>
#include <plib/sm.h>
#include <math.h>

/*
  Construct a sound scheduler and a mixer.
*/


slScheduler sched ( 44100 ) ;

int main ( int, char ** )
{
  sched . setSafetyMargin ( 0.5 ) ;

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




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

int main ( int, char ** )
{
  mixer . setMasterVolume ( 100 ) ;
  sched . setSafetyMargin ( 0.128f ) ;

  /* Just for fun, let's make a one second synthetic engine sample... */

  Uchar buffer [ 8000 ] ;

  for ( int i = 0 ; i < 8000 ; i++ )
  {
    /* Sum some sin waves and convert to range 0..1 */

    float level = (float)( sin ( (double) i * 2.0 * EX_PI / (8000.0/ 50.0) ) +
                           sin ( (double) i * 2.0 * EX_PI / (8000.0/149.0) ) +
                           sin ( (double) i * 2.0 * EX_PI / (8000.0/152.0) ) +
                           sin ( (double) i * 2.0 * EX_PI / (8000.0/192.0) )
                         ) / 8.0f + 0.5f ;

    /* Convert to unsigned byte */

    buffer [ i ] = (Uchar) ( level * 255.0f ) ;
  }

  /* Set up four samples and a loop */

  slSample  *s = new slSample ( buffer, 8000 ) ;
  slSample *s1 = new slSample ( "scream.ub", & sched ) ;
  slSample *s2 = new slSample ( "zzap.wav" , & sched ) ;
  slSample *s3 = new slSample ( "cuckoo.au", & sched ) ;
  slSample *s4 = new slSample ( "wheeee.ub", & sched ) ;

  /* Mess about with some of the samples... */

  s1 -> adjustVolume ( 10.0f  ) ;
  s2 -> adjustVolume ( 0.5f  ) ;
  s3 -> adjustVolume ( 0.2f  ) ;
  s3 -> adjustVolume ( 0.3f  ) ;

  /* Play the engine sample continuously. */

  sched . loopSample ( s ) ;
  sched . loopSample ( s4 ) ;

  int tim = 0 ;  /* My periodic event timer. */

  while ( SL_TRUE )
  {
    tim++ ;  /* Time passes */

    if ( tim % 140 == 0 ) sched.playSample ( s1 ) ;

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

    /*
      This would normally be called just before the graphics buffer swap
      - but it could be anywhere where it's guaranteed to get called
      fairly often.
    */

    sched . update () ;
  }

  return 0 ;
}


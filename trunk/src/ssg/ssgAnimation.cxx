
#include "ssgLocal.h"

void ssgTimedSelector::copy_from ( ssgTimedSelector *src, int clone_flags )
{
  ssgSelector::copy_from ( src, clone_flags ) ;

  running = src -> running ;
  mode    = src -> mode    ;

  start_time    = src -> start_time ;
  pause_time    = src -> pause_time ;
  loop_time     = src ->  loop_time ;

  for ( int i = 0 ; i < 32 ; i++ )
    times [ i ]  = src -> times [ i ] ;

  curr  = src -> curr  ;
  start = src -> start ;
  end   = src -> end   ;
}


ssgBase *ssgTimedSelector::clone ( int clone_flags )
{
  ssgTimedSelector *b = new ssgTimedSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTimedSelector::ssgTimedSelector (void)
{
  type |= SSG_TYPE_TIMEDSELECTOR ;
  select ( 1 ) ;
  running = SSG_ANIM_STOP ;
  mode = SSG_ANIM_SHUTTLE ;
  start_time = pause_time = 0.0f ;
  loop_time = 1.0f ;

  for ( int i = 0 ; i < 32 ; i++ )
    times [ i ] = 1.0f ;

  curr = start = end = 0 ;
}


void ssgTimedSelector::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  compute_loop_time () ;
  selectStep ( getStep () ) ;
  ssgSelector::cull ( f, m, test_needed ) ;
}


void ssgTimedSelector::hot ( sgVec3 sp, sgMat4 m, int test_needed )
{
  selectStep ( getStep () ) ;
  ssgSelector::hot ( sp, m, test_needed ) ;
}


void ssgTimedSelector::isect ( sgSphere *sp, sgMat4 m, int test_needed )
{
  selectStep ( getStep () ) ;
  ssgSelector::isect ( sp, m, test_needed ) ;
}


int ssgTimedSelector::getStep ()
{
  float t = (float) ssgGetFrameCounter () ;

  if ( running == SSG_ANIM_STOP || running == SSG_ANIM_PAUSE )
    return curr ;

  /* SSG_ANIM_START */

  t -= start_time ;  /* t is time since start of run */

  if ( mode == SSG_ANIM_ONESHOT )
  {
    if ( t >= loop_time )
    {
      running = SSG_ANIM_STOP ;
      return end ;
    }
  }
  else
  if ( mode == SSG_ANIM_SHUTTLE )      
  {
    /* Compute time since start of this loop */

    t = t - floor ( t / loop_time ) * loop_time ;
  }
  else
  if ( mode == SSG_ANIM_SWING )      
  {
    /* Compute time since start of this swing loop */

    t = t - floor ( t / (2.0f * loop_time) ) * (2.0f * loop_time) ;

    /* Are we on the reverse part of the loop? */
    if ( t >= loop_time )
      t = 2.0f * loop_time - t ;
  }

  int k ;

  for ( k = start ; t > 0.0f && k <= end ; k++ )
    t -= times [ k ] ;

  k-- ;

  if ( k < start ) k = start ;
  if ( k > end   ) k =   end ;

   curr = k ;
   return curr ;
}



ssgTimedSelector::~ssgTimedSelector (void)
{
  fprintf(stderr,"In ssgTimedSelectors' destructor.\n" ) ;
  print () ;
}



int ssgTimedSelector::load ( FILE *fd )
{
  _ssgReadInt   ( fd, (int *) & running ) ;
  _ssgReadInt   ( fd, (int *) & mode    ) ;
  _ssgReadFloat ( fd, & start_time ) ;
  _ssgReadFloat ( fd, & pause_time ) ;
  _ssgReadFloat ( fd, & loop_time  ) ;
  _ssgReadFloat ( fd, 32, times    ) ;
  _ssgReadInt   ( fd, & curr  ) ;
  _ssgReadInt   ( fd, & start ) ;
  _ssgReadInt   ( fd, & end   ) ;

  return ssgSelector::load(fd) ;
}

int ssgTimedSelector::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, (int) running ) ;
  _ssgWriteInt   ( fd, (int) mode    ) ;
  _ssgWriteFloat ( fd, start_time ) ;
  _ssgWriteFloat ( fd, pause_time ) ;
  _ssgWriteFloat ( fd, loop_time  ) ;
  _ssgWriteFloat ( fd, 32, times  ) ;
  _ssgWriteInt   ( fd, curr  ) ;
  _ssgWriteInt   ( fd, start ) ;
  _ssgWriteInt   ( fd, end   ) ;

  return ssgSelector::save(fd) ;
}



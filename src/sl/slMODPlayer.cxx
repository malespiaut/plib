
#include "sl.h"
#include "slMODPrivate.h"
#include "slMODfile.h"

int slMODPlayer::preempt ( int delay )
{
  return slPlayer::preempt ( delay ) ;
}

void slMODPlayer::init ( char *fname )
{
  mf = new MODfile ( fname, slScheduler::getCurrent()->getRate (), SL_FALSE ) ;
}

slMODPlayer::~slMODPlayer ()
{
  delete mf ;
}

void slMODPlayer::skip ( int /* nframes */ )
{
}


void slMODPlayer::low_read ( int nframes, Uchar *dst )
{
  if ( isWaiting() ) start () ;

  int need_bytes = nframes ;
  int all_done = 0 ;

  while ( need_bytes > 0 && !all_done )
  {
    int new_bytes = dacioGetLen () ;

    /* Compute some more audio */

    while ( new_bytes == 0 && !all_done )
    {
      all_done = ! mf -> update () ;
      new_bytes = dacioGetLen () ;
    }

    /* How much did we get? */

    if ( new_bytes > need_bytes )   /* oops! Too much */
    {
      memcpy ( dst, dacioGetOutBuffer (), need_bytes ) ;
      dacioSubtract ( need_bytes ) ;
      dst += need_bytes ;
      need_bytes = 0 ;
    }
    else
    {
      memcpy ( dst, dacioGetOutBuffer (), new_bytes ) ;
      dacioEmpty () ;
      dst += new_bytes ;
      need_bytes -= new_bytes ;
    }
  }
  
  /* Pad with silence if not enough data */

  if ( need_bytes > 0 )
    memset ( dst, 128, need_bytes ) ;

  if ( all_done )
  {
    if ( replay_mode == SL_SAMPLE_ONE_SHOT )
      stop () ;
    else
      start () ;
  }
}




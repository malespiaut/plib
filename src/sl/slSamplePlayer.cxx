
#include "sl.h"

int slSamplePlayer::preempt ( int delay )
{
  slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_PREEMPTED, magic ) ;

  return slPlayer::preempt ( delay ) ;
}

slSamplePlayer::~slSamplePlayer ()
{
  if ( sample )
    sample -> unRef () ;

  slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_COMPLETE, magic ) ;
}

void slSamplePlayer::skip ( int nframes )
{
  if ( nframes < lengthRemaining )
  {
    lengthRemaining -= nframes ;
    bufferPos       += nframes ;
  }
  else 
  if ( replay_mode == SL_SAMPLE_LOOP )
  {
    slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_LOOPED, magic ) ;

    nframes -= lengthRemaining ;

    while ( nframes >= sample->getLength () )
      nframes -= sample->getLength () ;

    lengthRemaining = sample->getLength() - nframes ;
    bufferPos = & ( sample->getBuffer() [ nframes ] ) ;
  }
  else
    stop () ;
}


void slSamplePlayer::low_read ( int nframes, Uchar *dst )
{
  if ( isWaiting() ) start () ;

  if ( bufferPos == NULL )  /* Run out of sample & no repeats */
  {
    memset ( dst, 0x80, nframes ) ;
    return ;
  }

  while ( SL_TRUE )
  {
    /*
      If we can satisfy this request in one read (with data left in
      the sample buffer ready for next time around) - then we are done...
    */

    if ( nframes < lengthRemaining )
    {
      memcpy ( dst, bufferPos, nframes ) ;
      bufferPos       += nframes ;
      lengthRemaining -= nframes ;
      return ;
    }

    memcpy ( dst, bufferPos, lengthRemaining ) ;
    bufferPos       += lengthRemaining ;
    dst             += lengthRemaining ;
    nframes         -= lengthRemaining ;
    lengthRemaining  = 0 ;

    if ( replay_mode == SL_SAMPLE_ONE_SHOT )
    {
      stop () ;
      memset ( dst, 0x80, nframes ) ;
      return ;
    }
    else
    {
      slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_LOOPED, magic ) ;
      start () ;
    }
  }
}




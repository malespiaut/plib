/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "sl.h"

char *__slPendingError = NULL ;

slScheduler *slScheduler::current = NULL ;

void slScheduler::init ()
{
  mixer = NULL ;
  
  mixer_buffer  = NULL ;
  spare_buffer0 = NULL ;
  spare_buffer1 = NULL ;
  spare_buffer2 = NULL ;
  
  current = this ;
  
  if ( notWorking () )
  {
    ulSetError ( UL_WARNING, "slScheduler: soundcard init failed." ) ;
    setError () ;
    return ;
  }
  
  if ( getBps() != 8 )
  {
    ulSetError ( UL_WARNING, "slScheduler: Needs a sound card that supports 8 bits per sample." ) ;
    setError () ;
    return ;
  }
  
  if ( getStereo() )
  {
    ulSetError ( UL_WARNING, "slScheduler: Needs a sound card that supports monophonic replay." ) ;
    setError () ;
    return ;
  }
  
  music = NULL ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    player [ i ] = NULL ;
  
  amount_left = 0 ;
  now = 0 ;
  num_pending_callbacks = 0 ;
  safety_margin = 1.0 ;
  
  initBuffers () ;
}

void slScheduler::initBuffers ()
{
  if ( notWorking () )
    return ;
  
  delete mixer_buffer ;
  delete spare_buffer0 ;
  delete spare_buffer1 ;
  delete spare_buffer2 ;
  
  mixer_buffer_size = getDriverBufferSize () ;
  
  mixer_buffer = new Uchar [ mixer_buffer_size ] ;
  memset ( mixer_buffer, 0x80, mixer_buffer_size ) ;
  
  spare_buffer0 = new Uchar [ mixer_buffer_size ] ;
  spare_buffer1 = new Uchar [ mixer_buffer_size ] ;
  spare_buffer2 = new Uchar [ mixer_buffer_size ] ;
}

slScheduler::~slScheduler ()
{
  if ( current == this )
    current = NULL ;
  
  delete mixer_buffer ;
  
  delete spare_buffer0 ;
  delete spare_buffer1 ;
  delete spare_buffer2 ;
}



void slScheduler::mixBuffer ( slPlayer *spa, slPlayer *spb )
{
  register int    l = mixer_buffer_size ;
  register Uchar *d = mixer_buffer ;
  
  register Uchar *a = spare_buffer0 ;
  register Uchar *b = spare_buffer1 ;
  
  spa -> read ( l, a ) ;
  spb -> read ( l, b ) ;
  
  while ( l-- ) *d++ = mix ( *a++, *b++ ) ;
}



void slScheduler::mixBuffer ( slPlayer *spa, slPlayer *spb,
                             slPlayer *spc )
{
  if ( notWorking () ) return ;
  
  register int    l = mixer_buffer_size ;
  register Uchar *d = mixer_buffer ;
  
  register Uchar *a = spare_buffer0 ;
  register Uchar *b = spare_buffer1 ;
  register Uchar *c = spare_buffer2 ;
  
  spa -> read ( l, a ) ;
  spb -> read ( l, b ) ;
  spc -> read ( l, c ) ;
  
  while ( l-- ) *d++ = mix ( *a++, *b++, *c++ ) ;
}


void slScheduler::realUpdate ( int dump_first )
{
  if ( notWorking () ) return ;
  
  if ( __slPendingError != NULL )
  {
    ulSetError ( UL_FATAL, "%s", __slPendingError ) ;
  }
  
  int i ;
  
  for ( i = 0 ; i < SL_MAX_SAMPLES ; i++ )
  {
    if ( player [ i ] == NULL )
      continue ;
    
    /* Clean up dead sample players */
    
    if ( player [ i ] -> isDone () )
    {
      if ( player [ i ] == music )
        music = NULL ;
      
      delete player [ i ] ;
      player [ i ] = NULL ;
      continue ;
    }
  }
  
  while ( secondsUsed() <= safety_margin )
  {
    slPlayer *psp [ 3 ] ;
    int       pri [ 3 ] ;
    
    pri [ 0 ] = pri [ 1 ] = pri [ 2 ] =  -1  ;
    
    for ( i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    {
      if ( player [ i ] == NULL )
        continue ;
      
      if ( player [ i ] -> isPaused () )
        continue ;
      
      int lowest = ( pri [0] <= pri [2] ) ?
        (( pri [0] <= pri [1] ) ? 0 : 1 ) :
      (( pri [1] <= pri [2] ) ? 1 : 2 ) ;
      
      if ( player[i]->getPriority() > pri[lowest] )
      {
        psp[lowest] = player[i] ;
        pri[lowest] = player[i]->getPriority() ;
      }
    }
    
    for ( i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    {
      if ( player [ i ] == NULL )
        continue ;
      
      if ( ! player [ i ] -> isPaused () &&
        player [ i ] != psp[0] &&
        player [ i ] != psp[1] &&
        player [ i ] != psp[2] )
      {
        player [ i ] -> preempt ( mixer_buffer_size ) ;
      }
    }
    
    if ( pri[0] < 0 )
    {
      memset ( mixer_buffer, 0x80, mixer_buffer_size ) ;
      amount_left = 0 ;
    }
    else
    if ( pri[1] < 0 )
      psp[0] -> read ( mixer_buffer_size, mixer_buffer ) ;
    else
    if ( pri[2] < 0 )
      mixBuffer ( psp[0], psp[1] ) ;
    else
      mixBuffer ( psp[0], psp[1], psp[2] ) ;
        
    if ( dump_first )
    {
      stop () ;
      dump_first = SL_FALSE ;
    }
    
    play ( mixer_buffer, mixer_buffer_size ) ;
    
    now += mixer_buffer_size ;
  }
  
  flushCallBacks () ;
}

void slScheduler::addCallBack ( slCallBack c, slSample *s, slEvent e, int m )
{
  if ( notWorking () ) return ;
  
  if ( num_pending_callbacks >= SL_MAX_CALLBACKS )
  {
    ulSetError ( UL_WARNING, "slScheduler: Too many pending callback events!" ) ;
    return ;
  }
  
  slPendingCallBack *p = & ( pending_callback [ num_pending_callbacks++ ] ) ;
  
  p -> callback = c ;
  p -> sample   = s ;
  p -> event    = e ;
  p -> magic    = m ;
}

void slScheduler::flushCallBacks ()
{
  if ( notWorking () ) return ;
  
  /*
  Execute all the callbacks that we accumulated
  in this iteration.
  
    This is done at the end of 'update' to reduce the risk
    of nasty side-effects caused by 'unusual' activities
    in the application's callback function.
  */
  
  while ( num_pending_callbacks > 0 )
  {
    slPendingCallBack *p = & ( pending_callback [ --num_pending_callbacks ] ) ;
    
    if ( p -> callback )
      (*(p->callback))( p->sample, p->event, p->magic ) ;
  }
}

void slScheduler::addSampleEnvelope ( slSample *s, int magic,
                                     int slot, slEnvelope *e,
                                     slEnvelopeType t)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> addEnvelope ( slot, e, t ) ;
}

void slScheduler::resumeSample ( slSample *s, int magic)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> resume () ;
}

void slScheduler::pauseSample ( slSample *s, int magic)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> pause () ;
}

void slScheduler::stopSample ( slSample *s, int magic )
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> stop () ;
}

int slScheduler::loopSample ( slSample *s, int pri,
                             slPreemptMode mode,
                             int magic, slCallBack cb )
{
  if ( notWorking () ) return -1 ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      player [ i ] = new slSamplePlayer ( s, SL_SAMPLE_LOOP, pri, mode, magic, cb ) ;
      return i ;
    }
    
  return -1 ;
}

int slScheduler::playSample ( slSample *s, int pri,
                             slPreemptMode mode,
                             int magic, slCallBack cb)
{
  if ( notWorking () ) return SL_FALSE ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      player [ i ] = new slSamplePlayer ( s, SL_SAMPLE_ONE_SHOT, pri, mode, magic, cb ) ;
      return SL_TRUE ;
    }
    
  return SL_FALSE ;
}

void slScheduler::addMusicEnvelope ( int magic,
                                    int slot, slEnvelope *e,
                                    slEnvelopeType t)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> addEnvelope ( slot, e, t ) ;
}

void slScheduler::resumeMusic ( int magic)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> resume () ;
}

void slScheduler::pauseMusic ( int magic)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> pause () ;
}

void slScheduler::stopMusic ( int magic )
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
  {
    music -> stop () ;
    
    for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
      if ( player [ i ] == music )
        player [ i ] = NULL ;
      
    delete music ;
    music = NULL ;
  }
}

int slScheduler::loopMusic ( char *fname, int pri,
                            slPreemptMode mode,
                            int magic, slCallBack cb )
{
  if ( notWorking () ) return -1 ;
  
  if ( music != NULL )
  {
    ulSetError ( UL_WARNING, "slScheduler: Can't play two music tracks at once." ) ;
    return -1 ;
  }      
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      music = new slMODPlayer ( fname, SL_SAMPLE_LOOP,
        pri, mode, magic, cb ) ;
      player [ i ] = music ;
      return i ;
    }
    
  return -1 ;
}

int slScheduler::playMusic ( char *fname, int pri,
                            slPreemptMode mode,
                            int magic, slCallBack cb )
{
  if ( notWorking () ) return SL_FALSE ;
  
  if ( music != NULL )
  {
    ulSetError ( UL_WARNING, "slScheduler: Can't play two music tracks at once." ) ;
    return SL_FALSE ;
  }      
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      music = new slMODPlayer ( fname, SL_SAMPLE_ONE_SHOT,
        pri, mode, magic, cb ) ;
      player [ i ] = music ;
      
      return SL_TRUE ;
    }
    
  return SL_FALSE ;
}




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
    fprintf ( stderr, "slScheduler: soundcard init failed.\n" ) ;
    setError () ;
    return ;
  }

  if ( getBps() != 8 )
  {
    fprintf ( stderr, "slScheduler: Needs a sound card that supports 8 bits per sample.\n" ) ;
    setError () ;
    return ;
  }

  if ( getStereo() )
  {
    fprintf ( stderr, "slScheduler: Needs a sound card that supports monophonic replay.\n" ) ;
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
    fprintf ( stderr, "%s", __slPendingError ) ;
    exit ( 1 ) ;
  }

  int i ;

  while ( secondsUsed() <= safety_margin )
  {
    slPlayer *psp [ 3 ] ;
    int       pri [ 3 ] ;

    pri [ 0 ] = pri [ 1 ] = pri [ 2 ] =  -1  ;

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
    fprintf ( stderr, "slScheduler: Too many pending callback events!\n" ) ;
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
    fprintf ( stderr, "slScheduler: Can't play two music tracks at once.\n" ) ;
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
    fprintf ( stderr, "slScheduler: Can't play two music tracks at once.\n" ) ;
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



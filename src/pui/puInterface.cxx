
#include "puLocal.h"

#define PUSTACK_MAX 100

static int currLiveInterface = -1 ;
static puInterface *liveInterfaceStack [ PUSTACK_MAX ] ;

void puPushLiveInterface ( puInterface *in )
{
  if ( currLiveInterface < PUSTACK_MAX )
    liveInterfaceStack [ ++currLiveInterface ] = in ;
  else
    fprintf ( stderr, "PUI: Too many live puInterfaces open at once!\n" ) ;
}

void  puPopLiveInterface ( void )
{
  if ( currLiveInterface > 0 )
    --currLiveInterface ;
  else 
    fprintf ( stderr, "PUI: Live puInterface stack is empty!\n" ) ;
}

int  puNoLiveInterface ( void )
{
  return currLiveInterface < 0 ;
}

puInterface *puGetUltimateLiveInterface ( void )
{
  if ( currLiveInterface < 0 )
  {
    fprintf ( stderr, "PUI: No Live Interface!\n" ) ;
    return NULL ;
  }

  return liveInterfaceStack [ 0 ] ;
}


puInterface *puGetBaseLiveInterface ( void )
{
  if ( currLiveInterface < 0 )
  {
    fprintf ( stderr, "PUI: No Live Interface!\n" ) ;
    return NULL ;
  }

  /*
    Work down the interface stack until you
    either get to the bottom or find a block
    in the form of a puDialogBox.
  */

  for ( int i = currLiveInterface ; i > 0 ; i-- )
    if ( liveInterfaceStack [ i ] -> getType () & PUCLASS_DIALOGBOX )
      return liveInterfaceStack [ i ] ; 

  return liveInterfaceStack [ 0 ] ;
}


puInterface::~puInterface ()
{
  puPopLiveInterface () ;

  for ( puObject *bo = dlist ; bo != NULL ; /* Nothing */ )
  {
    dlist = bo    ;
    bo = bo->next ;
    delete dlist  ;
  }
}



#include "ssgLocal.h"

void ssgState::copy_from ( ssgState *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;
  setExternalPropertyIndex ( src -> getExternalPropertyIndex () ) ;

  if ( src -> isTranslucent () )
    setTranslucent () ;
  else
    setOpaque () ;
}



ssgState::ssgState (void)
{
  type |= SSG_TYPE_STATE ;
  setOpaque () ;
  setExternalPropertyIndex ( 0 ) ;
}

ssgState::~ssgState (void) {}

int ssgState::load ( FILE *fd )
{
  _ssgReadInt ( fd, & translucent ) ;
  _ssgReadInt ( fd, & external_property_index ) ;
  return ssgBase::load ( fd ) ;
}


int ssgState::save ( FILE *fd )
{
  _ssgWriteInt ( fd, translucent ) ;
  _ssgWriteInt ( fd, external_property_index ) ;
  return ssgBase::save ( fd ) ;
}





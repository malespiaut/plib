
#include "ssgLocal.h"

void ssgTexture::print ( FILE *fd, char *ident )
{
  fprintf ( fd, "%s%s: whatever\n", ident, getTypeName () ) ;
}


int ssgTexture::load ( FILE *fd )
{
  _ssgReadUInt ( fd, & handle ) ;
  return ssgBase::load ( fd ) ;
}


int ssgTexture::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, handle ) ;
  return ssgBase::save ( fd ) ;
}


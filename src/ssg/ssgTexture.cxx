
#include "ssgLocal.h"

void ssgTexture::copy_from ( ssgTexture *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;
  setFilename ( src -> getFilename () ) ;
  handle = src -> getHandle () ;

  /*
    If clone_flags is SSG_CLONE_TEXTURE then we should
    extract the texels from OpenGL and recreate them.
    ...someday...
  */
}

ssgTexture *ssgTexture::clone ( int clone_flags )
{
  ssgTexture *b = new ssgTexture ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTexture::ssgTexture ()
{
  filename = NULL ;
  handle = 0 ;
}

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



#include "ssgLocal.h"

void ssgSimpleList::copy_from ( ssgSimpleList *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;

  delete list ;
  size_of = src -> getSizeOf () ;
  total   = src -> getNum () ;
  limit   = total ;
  list    = new char [ limit * size_of ] ;
  memcpy ( list, src->raw_get ( 0 ), limit * size_of ) ;
}

ssgBase *ssgSimpleList::clone ( int clone_flags )
{
  ssgSimpleList *b = new ssgSimpleList () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgNormalArray::clone ( int clone_flags )
{
  ssgNormalArray *b = new ssgNormalArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgVertexArray::clone ( int clone_flags )
{
  ssgVertexArray *b = new ssgVertexArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgTexCoordArray::clone ( int clone_flags )
{
  ssgTexCoordArray *b = new ssgTexCoordArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgColourArray::clone ( int clone_flags )
{
  ssgColourArray *b = new ssgColourArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


int ssgSimpleList::load ( FILE *fd )
{
  delete list ;
  _ssgReadUInt ( fd, &size_of ) ;
  _ssgReadUInt ( fd, &total   ) ;
  limit = total ;
  list = new char [ limit * size_of ] ;
  _ssgReadFloat ( fd, limit * size_of / sizeof(float), (float *)list ) ;
  return ! _ssgReadError () ;
}



int ssgSimpleList::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, size_of ) ;
  _ssgWriteUInt ( fd, total   ) ;
  _ssgWriteFloat( fd, limit * size_of / sizeof(float), (float *)list ) ;
  return ! _ssgWriteError () ;
}



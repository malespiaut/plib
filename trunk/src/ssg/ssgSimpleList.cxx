
#include "ssgLocal.h"

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



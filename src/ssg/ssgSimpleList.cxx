
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



 
void ssgVertexArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

	if ( how_much < 4 )
		return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  V%d) { %f, %f, %f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2] ) ;
}
 


 
void ssgNormalArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;
	if ( how_much < 4 )
		return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  N%d) { %f, %f, %f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2] ) ;
}
 


 
void ssgTexCoordArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;
	if ( how_much < 4 )
		return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  T%d) { S=%f, T=%f }\n", indent, i,
                     get(i)[0], get(i)[1] ) ;
}
 


 
void ssgColourArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;
	if ( how_much < 4 )
		return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  C%d) { R=%f, G=%f, B=%f, A=%f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2], get(i)[3] ) ;
}
 


void ssgSimpleList::print ( FILE *fd, char *indent, int how_much )
{
  ssgBase::print ( fd, indent, how_much ) ;

  fprintf ( fd, "%s  Total # items = %d\n", indent, total ) ;

	if ( how_much < 3 )
		return;

  fprintf ( fd, "%s  Size of items = %d bytes\n", indent, size_of ) ;
}
 


int ssgSimpleList::load ( FILE *fd )
{
  delete list ;
  _ssgReadUInt ( fd, &size_of ) ;
  _ssgReadUInt ( fd, &total   ) ;
  limit = total ;
  list = new char [ limit * size_of ] ;
	assert(list!=NULL);
  _ssgReadFloat ( fd, limit * size_of / sizeof(float), (float *)list ) ;
  return ! _ssgReadError () ;
}



int ssgSimpleList::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, size_of ) ;
  _ssgWriteUInt ( fd, total   ) ;
  _ssgWriteFloat( fd, total * size_of / sizeof(float), (float *)list ) ;
  return ! _ssgWriteError () ;
}



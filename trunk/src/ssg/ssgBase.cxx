
#include "ssgLocal.h"

void *ssgBase::operator new ( size_t sz )
{
  return malloc ( sz ) ;
}

void ssgBase::operator delete ( void *ptr )
{
  free ( ptr ) ;
}

void ssgBase::copy_from ( ssgBase *src, int /* clone_flags */ )
{
  type  = src -> getType () ;
  spare = src -> getSpare () ;
  refc  = 0 ;
}


ssgBase *ssgBase::clone ( int /* clone_flags */ )
{
  fprintf ( stderr, "SSG: Can't clone abstract SSG class objects\n" ) ;
  assert ( FALSE ) ;
  return NULL ;
}

ssgBase:: ssgBase (void)
{
  spare = refc = 0 ;
  type = SSG_TYPE_BASE ;
}

ssgBase::~ssgBase (void)
{
  deadBeefCheck () ;
  assert ( refc == 0 ) ;

  /*
    Set the type of deleted nodes to 0xDeadBeef so we'll
    stand a chance of detecting re-use of deleted nodes.
  */

  type = (int) 0xDeadBeef ;
}

void ssgBase::zeroSpareRecursive (){ zeroSpare () ; }
void ssgBase::zeroSpare ()         { spare = 0    ; }
void ssgBase::incSpare  ()         { spare++      ; }
void ssgBase::setSpare  ( int ss ) { spare = ss   ; }
int  ssgBase::getSpare  ()         { return spare ; }


void ssgBase::print ( FILE *fd, char *indent )
{
  fprintf ( fd, "%s%s: Ref Count=%d\n", indent, getTypeName(), getRef () ) ;
  deadBeefCheck () ;
}

int ssgBase::load ( FILE *fd )
{ 
  _ssgAddToList ( getSpare(), this ) ;
  return ! _ssgReadError () ;
}

int ssgBase::save ( FILE *fd )
{
  setSpare ( _ssgGetNextInstanceKey () ) ;
  return ! _ssgWriteError () ;
}




#include "ssgLocal.h"

void ssgBase::copy_from ( ssgBase *src, int /* clone_flags */ )
{
  type  = src -> getType () ;
  spare = src -> getSpare () ;
  refc  = 0 ;
}


ssgBase:: ssgBase (void)
{
  spare = refc = 0 ;
  type = SSG_TYPE_BASE ;
}

ssgBase::~ssgBase (void)
{
}

void ssgBase::zeroSpareRecursive (){ zeroSpare () ; }
void ssgBase::zeroSpare ()         { spare = 0    ; }
void ssgBase::incSpare  ()         { spare++      ; }
void ssgBase::setSpare  ( int ss ) { spare = ss   ; }
int  ssgBase::getSpare  ()         { return spare ; }


void ssgBase::print ( FILE *fd, char *indent )
{
  fprintf ( fd, "%s%s: Ref Count=%d\n", indent, getTypeName(), getRef () ) ;
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



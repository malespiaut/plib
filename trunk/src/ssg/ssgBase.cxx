
#include "ssgLocal.h"

static int next_unique_id = 1 ;

void *ssgBase::operator new ( size_t sz )
{
  return malloc ( sz ) ;
}

void ssgBase::operator delete ( void *ptr )
{
  free ( ptr ) ;
}

void ssgBase::copy_from ( ssgBase *src, int clone_flags )
{
  // type  = src -> getType () ; - cannot change the virtual function table...
  spare = src -> getSpare () ;
  // refc  = 0 ; - okay?

  if ( clone_flags & SSG_CLONE_USERDATA )
    setUserData ( src -> getUserData () ) ;
  else
    setUserData ( NULL ) ;

  setName ( src -> getName () ) ;
}



ssgBase *ssgBase::clone ( int /* clone_flags */ )
{
  ulSetError ( UL_FATAL, "SSG: Can't clone abstract SSG class objects" ) ;
  return NULL ;
}

ssgBase:: ssgBase (void)
{
  spare = refc = 0 ;
  type      = SSG_TYPE_BASE ;
  unique    = next_unique_id++ ;
  user_data = NULL ;
  name      = NULL ;
}

ssgBase::~ssgBase (void)
{
  ssgDeRefDelete ( user_data ) ;                                              

  deadBeefCheck () ;
  assert ( refc == 0 ) ;

  delete name ;

  /*
    Set the type of deleted nodes to 0xDeadBeef so we'll
    stand a chance of detecting re-use of deleted nodes.
  */

  type = (int) 0xDeadBeef ;
}

 
void ssgBase::setName ( const char *nm )
{
  delete name ;
 
  if ( nm == NULL )
    name = NULL ;
  else
  {
    name = new char [ strlen ( nm ) + 1 ] ;
    strcpy ( name, nm ) ;
  }
}
                                                                                
void ssgBase::zeroSpareRecursive (){ zeroSpare () ; }
void ssgBase::zeroSpare ()         { spare = 0    ; }
void ssgBase::incSpare  ()         { spare++      ; }
void ssgBase::setSpare  ( int ss ) { spare = ss   ; }
int  ssgBase::getSpare  ()         { return spare ; }


void ssgBase::print ( FILE *fd, char *indent, int how_much )
{

  if ( how_much > 2 )
  {	fprintf ( fd, "%s%s: Ref Count=%d\n", indent, getTypeName(), getRef () ) ;
		fprintf ( fd, "%s  Name = \"%s\"\n",  indent, getPrintableName() ) ;
	}
	else
		fprintf ( fd, "%s%s: Name=%s\n", indent, getTypeName(), getPrintableName() ) ;
	if ( how_much > 1 )
    fprintf ( fd, "%s  Userdata = %p\n",  indent, getUserData() ) ;
  deadBeefCheck () ;
}

int ssgBase::load ( FILE *fd )
{ 
  delete name ;
  name = NULL ;
  _ssgReadString ( fd, &name ) ;
  return ! _ssgReadError () ;
}

int ssgBase::save ( FILE *fd )
{
  _ssgWriteString ( fd, name ) ;
  return ! _ssgWriteError () ;
}



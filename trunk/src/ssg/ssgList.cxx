
#include "ssgLocal.h"

ssgList::ssgList ( int init )
{
  total = 0 ;
  entity_list = new ssgEntity * [ limit = (init <= 0) ? 1 : init ] ;
}


ssgKidList::ssgKidList ( int init ) : ssgList ( init )
{
}


ssgList::~ssgList (void)
{
  removeAllEntities () ;
  delete [] entity_list ;
}


ssgKidList::~ssgKidList (void)
{
}


void ssgList::addEntity ( ssgEntity *entity )
{
entity->deadBeefCheck() ;
  sizeChk () ;
  entity_list [ total++ ] = entity ;
}

void ssgKidList::addEntity ( ssgEntity *entity )
{
entity->deadBeefCheck() ;
  entity -> ref () ;
  ssgList::addEntity ( entity ) ;
}

void ssgList::sizeChk (void)
{
  /* Room for one more Entity? */

  if ( total >= limit )
  {
    limit += limit ;
    ssgEntity **nlist = new ssgEntity * [ limit ] ;
    memmove ( nlist, entity_list, sizeof(ssgEntity *) * total ) ;
    delete [] entity_list ;
    entity_list = nlist ;
  }
}


int ssgList::searchForEntity ( ssgEntity *entity )
{
  for ( unsigned int i = 0 ; i < total ; i++ )
    if ( entity_list [ i ] == entity )
      return (int) i ;

  return -1 ;
}


void ssgList::removeAllEntities ()
{
  while ( total > 0 )
    removeEntity ( (unsigned int) 0 ) ;
}

void ssgList::removeEntity ( unsigned int n )
{
entity_list[n]->deadBeefCheck();
  memmove ( &(entity_list[n]), &(entity_list[n+1]), sizeof(ssgEntity *) * (total-n-1) ) ;
  total-- ;

  if ( next >= n )
    next-- ;
}


void ssgKidList::removeEntity ( unsigned int n )
{
entity_list[n]->deadBeefCheck();
  ssgEntity *e = entity_list [ n ] ;

  ssgList::removeEntity ( n ) ;

  e -> deadBeefCheck () ;
  ssgDeRefDelete ( e ) ;
}





#include "ssgLocal.h"

void ssgSelector::copy_from ( ssgSelector *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;

  selection = src -> getSelect () ;
}


ssgBase *ssgSelector::clone ( int clone_flags )
{
  ssgSelector *b = new ssgSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgSelector::ssgSelector (void)
{
  type |= SSG_TYPE_SELECTOR ;
  selection = 0xFFFFFFFF ;
}

ssgSelector::~ssgSelector (void)
{
}

void ssgSelector::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection & ( 1<<s ) )
      e -> cull ( f, m, cull_result != SSG_INSIDE ) ;
}

void ssgSelector::hot ( sgVec3 sp, sgMat4 m, int test_needed )
{
  int hot_result = hot_test ( sp, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection & ( 1<<s ) )
      e -> hot ( sp, m, hot_result != SSG_INSIDE ) ;

  _ssgPopPath () ;
}


void ssgSelector::isect ( sgSphere *sp, sgMat4 m, int test_needed )
{
  int isect_result = isect_test ( sp, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection & ( 1<<s ) )
      e -> isect ( sp, m, isect_result != SSG_INSIDE ) ;

  _ssgPopPath () ;
}


int ssgSelector::load ( FILE *fd )
{
  _ssgReadUInt ( fd, & selection ) ;
  return ssgBranch::load(fd) ;
}

int ssgSelector::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, selection ) ;
  return ssgBranch::save(fd) ;
}



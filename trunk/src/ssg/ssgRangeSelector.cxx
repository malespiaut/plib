
#include "ssgLocal.h"

void ssgRangeSelector::copy_from ( ssgRangeSelector *src, int clone_flags )
{
  ssgSelector::copy_from ( src, clone_flags ) ;

  additive = src -> isAdditive () ;

  for ( unsigned int i = 0 ; i < 33 ; i++ )
    rng_list [ i ] = src -> getRange ( i ) ;
}


ssgRangeSelector *ssgRangeSelector::clone ( int clone_flags )
{
  ssgRangeSelector *b = new ssgRangeSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgRangeSelector::ssgRangeSelector (void)
{
  type |= SSG_TYPE_RANGESELECTOR ;
  additive = FALSE ;
  rng_list[0] = 0.0f ;

  for ( int i = 1 ; i < 33 ; i++ )
    rng_list[i] = SG_MAX ;
}

ssgRangeSelector::~ssgRangeSelector (void)
{
}

void ssgRangeSelector::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  float range = sgLengthVec3 ( m [ 3 ] ) ;

  if ( range < rng_list [ 0 ] )  /* Too close */
  {
    select ( 0 ) ;
    return ;
  }

  unsigned int sel = 0 ;

  for ( int i = 1 ; i < 33 ; i++ )
  {
    ssgEntity *e = getKid ( i-1 ) ;

    if ( e == NULL || rng_list [ i ] == SG_MAX )
    {
      select ( 0 ) ;
      return ;
    }

    if ( e != NULL && range < rng_list [ i ] )
    {
      e -> cull ( f, m, cull_result != SSG_INSIDE ) ;
      sel |= 1 << (i-1) ;

      if ( ! additive )
      {
        selectStep ( i-1 ) ;
        return ;
      }
    }
  }

  select ( sel ) ;
}


void ssgRangeSelector::hot ( sgVec3 sp, sgMat4 m, int test_needed )
{
  if ( additive )
    ssgBranch::hot ( sp, m, test_needed ) ;
  else
  {
    /* No point in testing this node since it only has one child */

    _ssgPushPath ( this ) ;

    ssgEntity *e = getKid ( 0 ) ;

    if ( e != NULL )
      e -> hot ( sp, m, test_needed ) ;

    _ssgPopPath () ;
  }
}


void ssgRangeSelector::isect ( sgSphere *sp, sgMat4 m, int test_needed )
{
  if ( additive )
    ssgBranch::isect ( sp, m, test_needed ) ;
  else
  {
    /* No point in testing this node since it only has one child */

    _ssgPushPath ( this ) ;

    ssgEntity *e = getKid ( 0 ) ;

    if ( e != NULL )
      e -> isect ( sp, m, test_needed ) ;

    _ssgPopPath () ;
  }
}



int ssgRangeSelector::load ( FILE *fd )
{
  _ssgReadInt   ( fd, & additive ) ;
  _ssgReadFloat ( fd, 33, rng_list ) ;
  return ssgSelector::load(fd) ;
}

int ssgRangeSelector::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, additive ) ;
  _ssgWriteFloat ( fd, 33, rng_list ) ;
  return ssgSelector::save(fd) ;
}



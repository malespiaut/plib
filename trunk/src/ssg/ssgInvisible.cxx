
#include "ssgLocal.h"

void ssgInvisible::copy_from ( ssgInvisible *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgInvisible::clone ( int clone_flags )
{
  ssgInvisible *b = new ssgInvisible ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



ssgInvisible::ssgInvisible (void)
{
  type |= SSG_TYPE_INVISIBLE ;
}


ssgInvisible::~ssgInvisible (void)
{
}


void ssgInvisible::cull ( sgFrustum *, sgMat4, int /* test_needed */ )
{
}



int ssgInvisible::load ( FILE *fd )
{
  return ssgBranch::load(fd) ;
}

int ssgInvisible::save ( FILE *fd )
{
  return ssgBranch::save(fd) ;
}


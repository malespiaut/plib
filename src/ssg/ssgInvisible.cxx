
#include "ssgLocal.h"


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


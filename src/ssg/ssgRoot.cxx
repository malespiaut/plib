
#include "ssgLocal.h"

ssgRoot::ssgRoot (void)
{
  type |= SSG_TYPE_ROOT ;
}

ssgRoot::~ssgRoot (void)
{
}


int ssgRoot::load ( FILE *fd )
{
  return ssgBranch::load(fd) ;
}

int ssgRoot::save ( FILE *fd )
{
  return ssgBranch::save(fd) ;
}



